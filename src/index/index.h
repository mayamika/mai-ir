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
    std::vector<int32_t> term_offsets_, inv_index_offsets_, coord_ids_offsets_,
        coord_index_offsets_;

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
            return -1;
        }

        return it - term_offsets_.begin();
    }

public:
    Index(const std::filesystem::path& dir) : dir_(dir) {
        term_offsets_ = file::TermOffsetsFile(dir_).offsets();
        inv_index_offsets_ = file::InvIndexOffsetsFile(dir_).offsets();
        coord_ids_offsets_ = file::CoordIdsOffsetsFile(dir_).offsets();
        coord_index_offsets_ = file::CoordIndexOffsetsFile(dir_).offsets();
    };

    std::vector<int32_t> posting_list(const std::string& term) const {
        int32_t term_id = find_term(term);
        if (term_id == -1) {
            return {};
        }
        int32_t offset = inv_index_offsets_[term_id];

        file::InvIndexFile index_file(dir_);
        return index_file.posting_list(offset);
    }

    std::vector<int32_t> coordinates(int32_t doc_id,
                                     const std::string& term) const {
        int32_t term_id = this->find_term(term);
        if (term_id == -1) {
            return {};
        }
        int32_t coord_ids_offset = coord_ids_offsets_[term_id];

        file::CoordIdsFile coord_ids_file(dir_);
        std::vector<int32_t> coord_ids = coord_ids_file.ids(coord_ids_offset);

        std::vector<int32_t> postings = this->posting_list(term);
        auto it = std::lower_bound(postings.begin(), postings.end(), doc_id);

        if (it == postings.end() || *it != doc_id) {
            return {};
        }

        int32_t doc_id_idx = it - postings.begin();
        int32_t coord_id = coord_ids[doc_id_idx];

        int32_t coord_index_offset = coord_index_offsets_[coord_id];

        file::CoordIndexFile coord_index_file(dir_);
        return coord_index_file.coordinates(coord_index_offset);
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
