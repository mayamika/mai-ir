CC := g++
GO := go

CXXSTD		?= c++17
CXXFLAGS	?= -Wall -Wextra -Werror

OUTPUT_DIR := build

$(OUTPUT_DIR):
	mkdir -p $@

define build-cxx
	$(CC) -o $@ -std=$(CXXSTD) $(CXXFLAGS)
endef

CREATE_INDEX_TARGET := $(OUTPUT_DIR)/build-index
CREATE_INDEX_SOURCES := src/create-index/main.cpp
CREATE_INDEX_HEADERS := src/byte/

$(CREATE_INDEX_TARGET): $(CREATE_INDEX_SOURCES) $(CREATE_INDEX_SOURCES) | $(OUTPUT_DIR)
	$(build-cxx) -I $(CREATE_INDEX_HEADERS) $(CREATE_INDEX_SOURCES)

TESTDATA_DIR	:= testdata
DB_FILE			?= $(TESTDATA_DIR)/db
INDEX_DIR		?= $(TESTDATA_DIR)/index

.PHONY: create-index
create-index: $(CREATE_INDEX_TARGET)
	$(CREATE_INDEX_TARGET) -i $(DB_FILE) -o $(INDEX_DIR)
