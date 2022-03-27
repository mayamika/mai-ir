#include <getopt.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <regex>
#include <set>
#include <string>

#include "byte/byte.h"

#define FATAL(text)                       \
    do {                                  \
        std::cerr << (text) << std::endl; \
        std::exit(1);                     \
    } while (1);

std::filesystem::path _src_index_path;
std::filesystem::path _output_dir;

void parse_options(int argc, char* argv[]) {
    option options[] = {
        {
            .name = "input",
            .has_arg = required_argument,
            .flag = 0,
            .val = 'i',
        },
        {
            .name = "output",
            .has_arg = required_argument,
            .flag = 0,
            .val = 'o',
        },
        {0, 0, 0, 0},
    };

    while (int val = getopt_long(argc, argv, "i:o:", options, nullptr)) {
        if (val == -1) {
            break;
        }

        switch (val) {
            case 'i':
                _src_index_path = std::filesystem::absolute(optarg);
                break;

            case 'o':
                _output_dir = std::filesystem::absolute(optarg);
                break;
        }
    }

    if (_output_dir == "") {
        FATAL("Output dir is not specified");
    }
}

std::regex _word_regex("[\\w\\'-]+");

std::set<std::string> tokenize(const std::string s) {
    auto begin = std::sregex_token_iterator(s.begin(), s.end(), _word_regex);
    auto end = std::sregex_token_iterator();

    return std::set<std::string>(begin, end);
}

struct Entry {
    std::string title;
    std::string original_title;
    std::string image;
    std::string description;
};

Entry parse_entry(const std::string& line) {
    Entry e;

    std::stringstream ss(line);
    char delim = '\t';

    std::string unused;
    std::getline(ss, unused, delim);            // id
    std::getline(ss, unused, delim);            // olang
    std::getline(ss, e.image, delim);           // image
    std::getline(ss, unused, delim);            // l_wikidata
    std::getline(ss, unused, delim);            // c_votecount
    std::getline(ss, unused, delim);            // c_popularity
    std::getline(ss, unused, delim);            // c_rating
    std::getline(ss, unused, delim);            // length
    std::getline(ss, e.title, delim);           // title
    std::getline(ss, e.original_title, delim);  // original
    std::getline(ss, unused, delim);            // alias
    std::getline(ss, unused, delim);            // l_renai
    std::getline(ss, e.description, delim);     // desc
    std::getline(ss, unused, delim);            // c_average

    return e;
}

void create_index(const std::filesystem::path& output_dir,
                  std::istream& src_index) {
    std::filesystem::create_directories(output_dir);
    std::ofstream db_file(output_dir / "db"),
        db_offsets_file(output_dir / "db-offsets"),
        count_file(output_dir / "count");

    std::string line;
    int32_t doc_id = 0;

    std::map<std::string, std::vector<uint32_t>> inv_index;

    std::vector<int32_t> db_offsets(1, 0);
    while (std::getline(src_index, line)) {
        auto e = parse_entry(line);
        auto tokens = tokenize(e.description);

        for (auto& v : tokens) {
            inv_index[v].push_back(doc_id);
        }
        db_file << line << '\n';
        db_offsets.push_back(db_file.tellp());

        ++doc_id;
    }
    db_offsets.pop_back();

    byte::write_int(count_file, doc_id);
    byte::write_vector(db_offsets_file, db_offsets);

    std::ofstream tokens_offsets_file(output_dir / "term-offsets"),
        tokens_file(output_dir / "terms");
    std::ofstream inv_index_offsets_file(output_dir / "inv-index-offsets"),
        inv_index_file(output_dir / "inv-index");

    std::vector<int32_t> tokens_offsets, index_offsets;

    for (auto& [token, doc_ids] : inv_index) {
        tokens_offsets.push_back(byte::write_string(tokens_file, token));
        index_offsets.push_back(byte::write_vector(inv_index_file, doc_ids));
    }
    std::exclusive_scan(tokens_offsets.begin(), tokens_offsets.end(),
                        tokens_offsets.begin(), 0);
    std::exclusive_scan(index_offsets.begin(), index_offsets.end(),
                        index_offsets.begin(), 0);

    byte::write_vector(tokens_offsets_file, tokens_offsets);
    byte::write_vector(inv_index_offsets_file, index_offsets);
}

int main(int argc, char* argv[]) {
    parse_options(argc, argv);

    std::cerr << "Creating index from " << _src_index_path << " at "
              << _output_dir << "..." << std::endl;

    if (_src_index_path == "") {
        create_index(_output_dir, std::cin);
    } else {
        std::ifstream file(_src_index_path);
        if (!file) {
            FATAL("Can't open source index file");
        }

        create_index(_output_dir, file);
    }

    return 0;
}
