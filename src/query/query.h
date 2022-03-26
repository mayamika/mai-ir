#ifndef QUERY_H
#define QUERY_H

#include <exception>
#include <stack>
#include <string>
#include <vector>

#include "index/index.h"

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

PostingList execute(const nindex::Index& index, const std::string& query);

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
