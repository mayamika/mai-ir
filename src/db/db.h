#ifndef DB_H
#define DB_H

#include <iostream>
#include <sstream>
#include <string>

namespace db {

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

}  // namespace db

#endif
