#ifndef FILE_H
#define FILE_H

#include <byte/byte.h>

#include <filesystem>
#include <fstream>

namespace file {

class CountFile {
    std::filesystem::path dir_;

public:
    CountFile(const std::filesystem::path& dir) : dir_(dir){};

    int32_t doc_count() const {
        std::ifstream count_file(dir_ / "count");
        return byte::read_int<int32_t>(count_file);
    }
};

class TermOffsetsFile {
    std::filesystem::path dir_;

public:
    TermOffsetsFile(const std::filesystem::path& dir) : dir_(dir){};

    std::vector<int32_t> offsets() const {
        std::ifstream offsets_file(dir_ / "term-offsets");
        return byte::read_vector<int32_t>(offsets_file);
    }
};

class TermsFile {
    std::filesystem::path dir_;

public:
    TermsFile(const std::filesystem::path& dir) : dir_(dir){};

    std::string term(int32_t offset) const {
        std::ifstream terms(dir_ / "terms");
        terms.seekg(offset);

        return byte::read_string(terms);
    }
};

class InvIndexOffsetsFile {
    std::filesystem::path dir_;

public:
    InvIndexOffsetsFile(const std::filesystem::path& dir) : dir_(dir){};

    std::vector<int32_t> offsets() const {
        std::ifstream offsets_file(dir_ / "inv-index-offsets");
        return byte::read_vector<int32_t>(offsets_file);
    }
};

class InvIndexFile {
    std::filesystem::path dir_;

public:
    InvIndexFile(const std::filesystem::path& dir) : dir_(dir){};

    std::vector<int32_t> posting_list(int32_t offset) const {
        std::ifstream index_file(dir_ / "inv-index");
        index_file.seekg(offset);

        return byte::read_vector<int32_t>(index_file);
    }
};

class DbOffsetsFile {
    std::filesystem::path dir_;

public:
    DbOffsetsFile(const std::filesystem::path& dir) : dir_(dir){};

    std::vector<int32_t> offsets() const {
        std::ifstream offsets_file(dir_ / "db-offsets");
        return byte::read_vector<int32_t>(offsets_file);
    }
};

struct DbEntry {
    std::string id;
    std::string title;
    std::string image;
    std::string description;
};

class DbFile {
    std::filesystem::path dir_;

    DbEntry parse_entry(const std::string& line) const {
        DbEntry e;

        std::stringstream ss(line);
        char delim = '\t';

        std::string unused;
        std::getline(ss, e.id, delim);           // id
        std::getline(ss, unused, delim);         // olang
        std::getline(ss, e.image, delim);        // image
        std::getline(ss, unused, delim);         // l_wikidata
        std::getline(ss, unused, delim);         // c_votecount
        std::getline(ss, unused, delim);         // c_popularity
        std::getline(ss, unused, delim);         // c_rating
        std::getline(ss, unused, delim);         // length
        std::getline(ss, e.title, delim);        // alias
        std::getline(ss, unused, delim);         // l_renai
        std::getline(ss, e.description, delim);  // desc
        std::getline(ss, unused, delim);         // c_average

        return e;
    }

public:
    DbFile(const std::filesystem::path& dir) : dir_(dir){};

    DbEntry entry(int32_t offset) const {
        std::ifstream db_file(dir_ / "db");
        db_file.seekg(offset);

        std::string line;
        std::getline(db_file, line);

        return parse_entry(line);
    }
};

}  // namespace file

#endif
