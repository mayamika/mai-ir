#ifndef INDEX_H
#define INDEX_H

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "byte/byte.h"
#include "file/file.h"

namespace nindex {

class Index {
    std::filesystem::path dir_;
    std::vector<int32_t> term_offsets_, inv_index_offsets_;

    std::string read_term(int32_t offset, const std::string& term) const {
        if (offset == -1) {
            return term;
        }

        file::TermsFile terms_file(dir_);
        return terms_file.term(offset);
    }

    int32_t find_term(const std::string& term) const {
        auto cmp = [&](int32_t l_offset, int32_t r_offset) {
            auto l = read_term(l_offset, term), r = read_term(r_offset, term);
            return l < r;
        };
        auto it = std::lower_bound(term_offsets_.begin(), term_offsets_.end(),
                                   -1, cmp);

        if (it == term_offsets_.end() || term != read_term(*it, "")) {
            throw std::logic_error("not found");
        }

        return it - term_offsets_.begin();
    }

public:
    Index(const std::filesystem::path& dir) : dir_(dir) {
        term_offsets_ = file::TermOffsetsFile(dir).offsets();
        inv_index_offsets_ = file::InvIndexOffsetsFile(dir).offsets();
    };

    std::vector<int32_t> posting_list(const std::string& term) const {
        int32_t term_id = find_term(term);
        int32_t offset = inv_index_offsets_[term_id];

        file::InvIndexFile index_file(dir_);
        return index_file.posting_list(offset);
    }

    std::vector<int32_t> all_docs() const {
        file::CountFile count_file(dir_);
        int32_t n = count_file.doc_count();

        std::vector<int32_t> doc_ids;
        doc_ids.reserve(n);

        for (int i = 0; i < n; i++) {
            doc_ids.push_back(i);
        }

        return doc_ids;
    }
};

}  // namespace nindex

#endif
