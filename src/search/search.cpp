#include "search.h"

#include "db/db.h"
#include "index/index.h"
#include "query/query.h"

void free_item(Item* i) {
    delete[] i->title;
    delete[] i->original_title;
    delete[] i->image;
    delete[] i->description;
}

void free_query_result(QueryResult* qr) {
    for (int i = 0; i < qr->count; ++i) {
        free_item(qr->items[i]);
    }
}

void copy_to_cstring(char** dst, std::string& src) {
    size_t n = src.size();
    *dst = new char[n + 1];
    (*dst)[n] = '\0';

    for (size_t i = 0; i < n; ++i) {
        (*dst)[i] = src[i];
    }
}

void read_doc(const std::filesystem::path dir, int32_t doc_id, Item* item) {
    std::ifstream db(dir / "db");

    std::string line;
    for (int32_t i = 1; i < doc_id; ++i) getline(db, line);

    getline(db, line);

    auto e = db::parse_entry(line);

    copy_to_cstring(&item->title, e.title);
    copy_to_cstring(&item->original_title, e.original_title);
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

    size_t n = posting_list.size();
    qr->count = n;
    qr->items = new Item*[n];

    for (size_t i = 0; i < n; ++i) {
        Item* item = new Item;
        int32_t doc_id = posting_list[i];

        read_doc(index_path, doc_id, item);

        qr->items[i] = item;
    }

    return 0;
}
