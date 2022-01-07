package search

// #cgo CXXFLAGS: -std=c++17 -Wall -Wextra -Werror
//
// #include "search.h"
// #include <stdlib.h>
import "C"

import (
	"errors"
	"unsafe"
)

type QueryResult struct {
	Title         string
	OriginalTitle string
	Image         string
	Description   string
}

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

func queryResultFromC(qr *C.QueryResult) *QueryResult {
	return &QueryResult{
		Title:         C.GoString(qr.title),
		OriginalTitle: C.GoString(qr.original_title),
		Image:         C.GoString(qr.image),
		Description:   C.GoString(qr.description),
	}
}
