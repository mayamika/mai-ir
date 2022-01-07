#include "search.h"

#include <cstring>

void free_query_result(QueryResult* qr) {
    delete[] qr->title;
    delete[] qr->original_title;
    delete[] qr->image;
    delete[] qr->description;
}

int search(QueryResult* qr, const char* index_path, const char* query) {
    qr->title = new char[256];
    strcpy(qr->title, index_path);

    qr->original_title = new char[256];
    strcpy(qr->original_title, query);

    qr->image = nullptr;
    qr->description = nullptr;

    return 0;
}
