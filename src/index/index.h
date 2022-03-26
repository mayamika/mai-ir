#ifndef INDEX_H
#define INDEX_H

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "byte/byte.h"

namespace nindex {

class Index {
    std::filesystem::path dir_;

    std::string read_term(int32_t offset, const std::string& term) const {
        if (offset == -1) {
            return term;
        }

        std::ifstream terms(dir_ / "tokens");
        terms.seekg(offset);

        return byte::read_string(terms);
    }

    int32_t find_term(const std::string& term) const {
        std::ifstream offsets_file(dir_ / "tokens-offsets");

        std::vector<int32_t> offsets = byte::read_vector<int32_t>(offsets_file);
        auto it = std::lower_bound(
            offsets.begin(), offsets.end(), -1, [&](int32_t lid, int32_t rid) {
                auto l = read_term(lid, term), r = read_term(rid, term);
                return l < r;
            });

        if (it == offsets.end() || term != read_term(*it, "")) {
            throw std::logic_error("not found");
        }

        return it - offsets.begin();
    }

public:
    Index(const std::filesystem::path& dir) : dir_(dir){};
    std::vector<int32_t> posting_list(const std::string& term) const {
        int32_t term_id = find_term(term);

        std::ifstream index_offsets_file(dir_ / "inv-index-offsets"),
            index_file(dir_ / "inv-index");

        std::vector<int32_t> offsets =
            byte::read_vector<int32_t>(index_offsets_file);

        int32_t offset = offsets[term_id];
        index_file.seekg(offset);

        std::vector<int32_t> posting_list =
            byte::read_vector<int32_t>(index_file);

        return posting_list;
    }

    int32_t doc_count() const {
        std::ifstream count_file(dir_ / "count");
        return byte::read_int<int32_t>(count_file);
    }

    std::vector<int32_t> all_docs() const {
        int32_t n = doc_count();
        std::vector<int32_t> ret;
        ret.reserve(n);

        for (int i = 1; i <= n; i++) {
            ret.push_back(i);
        }

        return ret;
    }
};

}  // namespace nindex

#endif