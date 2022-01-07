#ifndef SEARCH_H
#define SEARCH_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* title;
    char* original_title;
    char* image;
    char* description;
} QueryResult;

void free_query_result(QueryResult* qr);

int search(QueryResult* qr, const char* index_path, const char* query);

#ifdef __cplusplus
}
#endif

#endif
