#include <getopt.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <string>

#include "byte.h"

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

struct Entry {
    std::string title;
    std::string original_title;
    std::string description;
};

Entry parse_entry(const std::string& line) {
    Entry e;

    std::stringstream ss(line);
    char delim = '\t';

    std::string unused;
    getline(ss, unused, delim);            // id
    getline(ss, unused, delim);            // olang
    getline(ss, unused, delim);            // image
    getline(ss, unused, delim);            // l_wikidata
    getline(ss, unused, delim);            // c_votecount
    getline(ss, unused, delim);            // c_popularity
    getline(ss, unused, delim);            // c_rating
    getline(ss, unused, delim);            // length
    getline(ss, e.title, delim);           // title
    getline(ss, e.original_title, delim);  // original
    getline(ss, unused, delim);            // alias
    getline(ss, unused, delim);            // l_renai
    getline(ss, e.description, delim);     // desc
    getline(ss, unused, delim);            // c_average

    return e;
}

std::regex _word_regex("[\\w\\'-]+");

std::set<std::string> tokenize(const std::string s) {
    auto begin = std::sregex_token_iterator(s.begin(), s.end(), _word_regex);
    auto end = std::sregex_token_iterator();

    return std::set<std::string>(begin, end);
}

void create_index(const std::filesystem::path& output_dir,
                  std::istream& src_index) {
    std::filesystem::create_directories(output_dir);

    std::string line;
    int32_t doc_id = 1;

    std::map<std::string, std::vector<uint32_t>> inv_index;

    while (std::getline(src_index, line)) {
        Entry e = parse_entry(line);
        auto tokens = tokenize(e.description);

        for (auto& v : tokens) {
            inv_index[v].push_back(doc_id);
        }

        ++doc_id;
    }

    std::ofstream tokens_offsets_file(output_dir / "tokens-offsets"),
        tokens_file(output_dir / "tokens");
    std::ofstream inv_index_offsets_file(output_dir / "inv-index-offsets"),
        inv_index_file(output_dir / "inv-index");

    for (auto& [token, doc_ids] : inv_index) {
        byte::write_int(tokens_offsets_file,
                        byte::write_string(tokens_file, token));
        byte::write_int(inv_index_offsets_file,
                        byte::write_vector(inv_index_file, doc_ids));
    }
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
