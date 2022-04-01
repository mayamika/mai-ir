#include "query/query.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stack>
#include <string>

namespace query {

template <typename T>
T top(const std::stack<T>& args) {
    if (args.empty()) {
        throw BadSyntax();
    }

    return args.top();
}

template <typename T>
T pop_top(std::stack<T>& args) {
    auto r = top(args);
    args.pop();
    return r;
}

bool is_delim(char c) {
    return c == ' ';
}

bool is_operation(std::string s) {
    return s == "&&" || s == "||" || s == "!";
}

enum Operation {
    UNION,
    INTERSECTION,
    NEGATION,
    BRACE,
};

int priority(Operation op) {
    return op == UNION ? 1 : op == INTERSECTION ? 2 : op == NEGATION ? 3 : -1;
}

PostingList union_postings(const PostingList& lhs, const PostingList& rhs) {
    size_t size = lhs.size() + rhs.size();
    PostingList ret;
    ret.reserve(size);

    for (size_t i = 0, j = 0; i < lhs.size() && j < rhs.size();) {
        if (lhs[i] == rhs[j]) {
            ret.push_back(lhs[i]);
            i++, j++;
        } else if (lhs[i] < rhs[j]) {
            ret.push_back(lhs[i]);
            i++;
        } else {
            ret.push_back(rhs[j]);
            j++;
        }
    }

    return ret;
}

PostingList intersect_postings(const PostingList& lhs, const PostingList& rhs) {
    size_t size = std::min(lhs.size(), rhs.size());
    PostingList ret;
    ret.reserve(size);

    for (size_t i = 0, j = 0; i < lhs.size() && j < rhs.size();) {
        if (lhs[i] == rhs[j]) {
            ret.push_back(lhs[i]);
            i++, j++;
        } else if (lhs[i] < rhs[j]) {
            i++;
        } else {
            j++;
        }
    }

    return ret;
}

PostingList negate_postings(PostingList all_docs, const PostingList& v) {
    size_t size = all_docs.size() - v.size();
    PostingList ret;
    ret.reserve(size);

    size_t j = 0;
    for (size_t i = 0; i < all_docs.size(); ++i) {
        while (j != v.size() && v[j] < all_docs[i]) ++j;
        if (j != v.size() && v[j] == all_docs[i]) continue;

        ret.push_back(all_docs[i]);
    }

    return ret;
}

void process_op(const PostingList& all_docs, std::stack<PostingList>& args,
                Operation op) {
    PostingList r = pop_top(args), l;

    switch (op) {
        case UNION:
            l = pop_top(args);

            args.push(union_postings(l, r));
            break;
        case INTERSECTION:
            l = pop_top(args);

            args.push(intersect_postings(l, r));
            break;
        case NEGATION:
            args.push(negate_postings(all_docs, r));
            break;
        case BRACE:
            throw BadSyntax("unexpected brace");
            break;
    }
}

std::vector<int32_t> coords_posting_list(const nindex::Index& index,
                                         const std::vector<std::string>& terms,
                                         int32_t k) {
    std::vector<int32_t> docs = index.all_docs();
    for (auto& term : terms) {
        docs = intersect_postings(docs, index.posting_list(term));
    }

    std::vector<int32_t> posting_list;
    for (auto doc_id : docs) {
        std::vector<std::vector<int32_t>> coordinates(terms.size());
        for (size_t j = 0; j < terms.size(); ++j) {
            coordinates[j] = index.coordinates(doc_id, terms[j]);
        }

        bool found = false;

        for (size_t i = 0; i < coordinates[0].size(); ++i) {
            std::vector<int32_t> positions(coordinates.size(),
                                           std::numeric_limits<int32_t>::max());
            positions[0] = coordinates[0][i];

            for (size_t j = 1; j < positions.size(); ++j) {
                auto it =
                    std::upper_bound(coordinates[j].begin(),
                                     coordinates[j].end(), positions[j - 1]);
                if (it == coordinates[j].end()) {
                    break;
                }

                positions[j] = *it;
                if (positions[j] - positions.front() > k) {
                    break;
                }
            }

            if (positions.back() - positions.front() <= k) {
                found = true;
                break;
            }
        }

        if (found) {
            posting_list.push_back(doc_id);
        }
    }

    return posting_list;
}

int try_op(const std::string& q, size_t i, Operation& op) {
    if (q[i] == '!') {
        op = NEGATION;
        return 1;
    }
    if (i == q.size() - 1) {
        return 0;
    }

    auto sub = q.substr(i, 2);
    if (sub == "&&") {
        op = INTERSECTION;
        return 2;
    }
    if (sub == "||") {
        op = UNION;
        return 2;
    }

    return 0;
}

PostingList scan_quote(const nindex::Index& index, const std::string& s,
                       size_t& i) {
    std::vector<std::string> terms;
    size_t n = s.length();

    ++i;
    for (; i < n; ++i) {
        std::string term;

        while (i < n && isalnum(s[i])) {
            term += s[i++];
        }
        if (!term.empty()) {
            terms.push_back(term);
        }

        if (s[i] == '"') {
            break;
        }
    }

    if (i == n) {
        throw BadSyntax("unfinished quote");
    }
    if (terms.empty()) {
        throw BadSyntax("empty quote");
    }

    int32_t k = 0;

    ++i;
    while (i < n && isspace(s[i])) ++i;
    if (i < n && s[i] == '/') {
        ++i;
        while (i < n && isspace(s[i])) ++i;

        for (; i < n && isdigit(s[i]); i++) {
            k *= 10;
            k += s[i] - '0';
        }

        if (k < static_cast<int32_t>(terms.size() - 1)) {
            throw BadSyntax("quote distance is invalid");
        }
    }

    --i;
    k = std::max<int32_t>(k, terms.size() - 1);
    return coords_posting_list(index, terms, k);
}

PostingList execute(const nindex::Index& index, const std::string& query) {
    std::stack<PostingList> args;
    std::stack<Operation> ops;

    bool mb_intersect = false;

    for (size_t i = 0; i < query.length(); ++i) {
        char ch = query[i];
        if (is_delim(ch)) continue;

        if (ch == '(') {
            if (mb_intersect) {
                ops.push(INTERSECTION);
            }

            ops.push(BRACE);
            mb_intersect = false;
            continue;
        }

        if (ch == ')') {
            while (top(ops) != BRACE) {
                process_op(index.all_docs(), args, pop_top(ops));
            }

            ops.pop();
            mb_intersect = true;
            continue;
        }

        Operation op;
        int ok = try_op(query, i, op);

        if (ok) {
            i += ok - 1;

            while (!ops.empty() && priority(ops.top()) >= priority(op)) {
                process_op(index.all_docs(), args, pop_top(ops));
            }

            if (op == NEGATION && mb_intersect) {
                ops.push(INTERSECTION);
            }
            mb_intersect = false;
            ops.push(op);
            continue;
        }
        // Scan token.

        if (mb_intersect) {
            ops.push(INTERSECTION);
        }

        if (query[i] == '"') {
            args.push(scan_quote(index, query, i));
            continue;
        }

        std::string token;
        while (i < query.length() && isalnum(query[i])) {
            token += query[i++];
        }
        --i;

        if (token == "") {
            throw BadSyntax("term expected");
        }

        args.push(index.posting_list(token));
        mb_intersect = true;
    }

    while (!ops.empty()) {
        process_op(index.all_docs(), args, pop_top(ops));
    }
    return top(args);
}

}  // namespace query
