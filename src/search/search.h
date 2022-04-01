#ifndef SEARCH_H
#define SEARCH_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* id;
    char* title;
    char* image;
    char* description;
} Item;

typedef struct {
    int32_t count;
    Item** items;
} QueryResult;

void free_query_result(QueryResult* qr);

int search(QueryResult* qr, const char* index_path, const char* query);

#ifdef __cplusplus
}
#endif

#endif
