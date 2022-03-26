#include "query/query.h"

#include <algorithm>
#include <cmath>
#include <stack>
#include <string>

namespace query {

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

    for (size_t i = 0, j = 0; i < all_docs.size() && j < v.size();) {
        if (all_docs[i] == v[j]) {
            i++, j++;
        } else if (all_docs[i] < v[j]) {
            ret.push_back(all_docs[i]);
            i++;
        } else {
            j++;
        }
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
