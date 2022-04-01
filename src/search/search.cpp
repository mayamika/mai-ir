#include "search.h"

#include "file/file.h"
#include "index/index.h"
#include "query/query.h"

void free_item(Item* i) {
    delete[] i->id;
    delete[] i->title;
    delete[] i->image;
    delete[] i->description;
}

void free_query_result(QueryResult* qr) {
    for (int i = 0; i < qr->count; ++i) {
        free_item(qr->items[i]);
    }
}

void copy_to_cstring(char** dst, const std::string& src) {
    size_t n = src.size();
    *dst = new char[n + 1];
    (*dst)[n] = '\0';

    for (size_t i = 0; i < n; ++i) {
        (*dst)[i] = src[i];
    }
}

void copy_to_c(Item* item, const file::DbEntry& e) {
    copy_to_cstring(&item->id, e.id);
    copy_to_cstring(&item->title, e.title);
    copy_to_cstring(&item->image, e.image);
    copy_to_cstring(&item->description, e.description);
}

int search(QueryResult* qr, const char* index_path, const char* query) {
    nindex::Index idx(index_path);

    std::vector<int32_t> posting_list;

    try {
        posting_list = query::execute(idx, query);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    std::vector<int32_t> db_offsets = file::DbOffsetsFile(index_path).offsets();
    file::DbFile db(index_path);

    size_t n = posting_list.size();
    qr->count = n;
    qr->items = new Item*[n];

    for (size_t i = 0; i < n; ++i) {
        int32_t doc_id = posting_list[i];
        file::DbEntry entry = db.entry(db_offsets[doc_id]);

        Item* item = new Item;
        copy_to_c(item, entry);
        qr->items[i] = item;
    }

    return 0;
}
