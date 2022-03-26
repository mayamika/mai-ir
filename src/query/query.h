#ifndef QUERY_H
#define QUERY_H

#include <exception>
#include <stack>
#include <string>
#include <vector>

namespace query {

class BadSyntax : public std::exception {
    std::string what_;

public:
    BadSyntax() : what_("bad syntax"){};
    BadSyntax(const std::string& what) : what_("bad syntax: " + what){};

    virtual const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW {
        return what_.c_str();
    }
};

using PostingList = std::vector<int32_t>;

class Index {
public:
    virtual PostingList posting_list(const std::string& term) const = 0;
};

PostingList execute(const Index& index, const std::string& query);

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

}  // namespace query

#endif
