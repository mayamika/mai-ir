CC := g++
GO := go

CXXSTD		?= c++17
CXXFLAGS	?= -Wall -Wextra -Werror

OUTPUT_DIR := build

$(OUTPUT_DIR):
	mkdir -p $@

INCLUDE_PATH := ./src

define build-cxx
	$(CC) -o $@ -std=$(CXXSTD) $(CXXFLAGS) -I$(INCLUDE_PATH)
endef

define build-cxx-lib
	$(build-cxx) -fPIC -shared
endef

CGO_DIR := src/search/


BYTE_HEADERS 			:= src/byte/byte.h
INDEX_BOOLEAN_HEADERS 	:= src/index/boolean/boolean.h
QUERY_HEADERS 			:= src/query/query.h
QUERY_SOURCES			:= src/query/query.cpp

QUERY_LIB_TARGET := $(CGO_DIR)/libquery.so
QUERY_LIB_SOURCES := $(QUERY_SOURCES)
QUERY_LIB_HEADERS := $(QUERY_HEADERS) $(BYTE_HEADERS) $(INDEX_BOOLEAN_HEADERS)

$(QUERY_LIB_TARGET): $(QUERY_LIB_SOURCES) $(QUERY_LIB_HEADERS)
	$(build-cxx-lib) $(QUERY_LIB_SOURCES)

.PHONY: build-cgo
build-cgo: $(QUERY_LIB_TARGET)

CREATE_INDEX_TARGET := $(OUTPUT_DIR)/build-index
CREATE_INDEX_SOURCES := src/create-index/main.cpp
CREATE_INDEX_HEADERS := src/byte/

$(CREATE_INDEX_TARGET): $(CREATE_INDEX_SOURCES) $(CREATE_INDEX_HEADERS) | $(OUTPUT_DIR)
	$(build-cxx) $(CREATE_INDEX_SOURCES)

TESTDATA_DIR	:= testdata
DB_FILE			?= $(TESTDATA_DIR)/db
INDEX_DIR		?= $(TESTDATA_DIR)/index

.PHONY: create-index
create-index: $(CREATE_INDEX_TARGET)
	$(CREATE_INDEX_TARGET) -i $(DB_FILE) -o $(INDEX_DIR)
