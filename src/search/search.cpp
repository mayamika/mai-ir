#include "search.h"

#include "index/boolean/boolean.h"
#include "query/query.h"

void free_query_result(QueryResult* qr) {
    delete[] qr->title;
    delete[] qr->original_title;
    delete[] qr->image;
    delete[] qr->description;
}

int search(QueryResult* qr, const char* index_path, const char* query) {
    index::boolean::Index index(index_path);
    query::execute(index, query);

    qr->title = new char[256];

    qr->original_title = new char[256];

    qr->image = nullptr;
    qr->description = nullptr;

    return 0;
}
