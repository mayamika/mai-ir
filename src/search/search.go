package search

// #cgo CXXFLAGS: -std=c++17 -Wall -Wextra -Werror -I..
// #cgo LDFLAGS: -L${SRCDIR} -lquery
//
// #include "search.h"
// #include <stdlib.h>
import "C"

import (
	"errors"
	"unsafe"
)

type Item struct {
	Title         string
	OriginalTitle string
	Image         string
	Description   string
}

type QueryResult struct {
	Count int
	Items []*Item
}

//nolint: gocritic
func Search(indexPath, query string) (*QueryResult, error) {
	cIndexPath, cQuery := C.CString(indexPath), C.CString(query)
	defer func() {
		C.free(unsafe.Pointer(cIndexPath))
		C.free(unsafe.Pointer(cQuery))
	}()

	var cqr C.QueryResult
	code := C.search(&cqr, cIndexPath, cQuery)

	qr := queryResultFromC(&cqr)
	C.free_query_result(&cqr)

	if code != 0 {
		return nil, errors.New("unknown error")
	}

	return qr, nil
}

func itemFromC(i *C.Item) *Item {
	return &Item{
		Title:         C.GoString(i.title),
		OriginalTitle: C.GoString(i.original_title),
		Image:         C.GoString(i.image),
		Description:   C.GoString(i.description),
	}
}

func queryResultFromC(cqr *C.QueryResult) *QueryResult {
	qr := &QueryResult{
		Count: int(cqr.count),
	}
	qr.Items = make([]*Item, qr.Count)

	cItems := unsafe.Slice(cqr.items, qr.Count)
	for i := range qr.Items {
		qr.Items[i] = itemFromC(cItems[i])
	}

	return qr
}
