#include "query/query.h"

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
    auto v = lhs;
    v.resize(rhs.size());
    return v;
}

PostingList intersect_postings(const PostingList& lhs, const PostingList& rhs) {
    auto v = lhs;
    v.resize(rhs.size());
    return v;
}

PostingList negate_postings(const PostingList& v) {
    return v;
}

void process_op(std::stack<PostingList>& args, Operation op) {
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
            args.push(negate_postings(r));
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

PostingList execute(const Index& index, const std::string& query) {
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
                process_op(args, pop_top(ops));
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
                process_op(args, pop_top(ops));
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
        process_op(args, pop_top(ops));
    }
    return top(args);
}

}  // namespace query
