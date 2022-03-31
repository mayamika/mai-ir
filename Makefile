CC		:= g++
GO		:= go
DOCKER	:= docker

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

FILE_HEADERS			:= src/file/file.h
BYTE_HEADERS 			:= src/byte/byte.h
INDEX_HEADERS 			:= src/index/index.h
QUERY_HEADERS 			:= src/query/query.h
QUERY_SOURCES			:= src/query/query.cpp

QUERY_LIB_TARGET := $(OUTPUT_DIR)/libquery.so
QUERY_LIB_SOURCES := $(QUERY_SOURCES)
QUERY_LIB_HEADERS := $(QUERY_HEADERS) $(FILE_HEADERS) $(BYTE_HEADERS) $(INDEX_HEADERS)

$(QUERY_LIB_TARGET): $(QUERY_LIB_SOURCES) $(QUERY_LIB_HEADERS) | $(OUTPUT_DIR)
	$(build-cxx-lib) $(QUERY_LIB_SOURCES)

.PHONY: build-cgo
build-cgo: $(QUERY_LIB_TARGET)

CREATE_INDEX_TARGET := $(OUTPUT_DIR)/build-index
CREATE_INDEX_SOURCES := src/create-index/main.cpp
CREATE_INDEX_HEADERS := $(FILE_HEADERS) $(BYTE_HEADERS)

$(CREATE_INDEX_TARGET): $(CREATE_INDEX_SOURCES) $(CREATE_INDEX_HEADERS) | $(OUTPUT_DIR)
	$(build-cxx) $(CREATE_INDEX_SOURCES)

TESTDATA_DIR	:= testdata
DB_FILE			?= $(TESTDATA_DIR)/db
INDEX_DIR		?= $(TESTDATA_DIR)/index

.PHONY: create-index
create-index: $(CREATE_INDEX_TARGET)
	$(CREATE_INDEX_TARGET) -i $(DB_FILE) -o $(INDEX_DIR)

# Docker.

IMAGE_NAME		:= mai-ir
IMAGE_VERSION	?= latest
IMAGE_TAG		:= $(IMAGE_NAME):$(IMAGE_VERSION)

.PHONY: build-image
build-image:
	$(DOCKER) build -t $(IMAGE_TAG) .

.PHONY: run-image
run-image:
	$(DOCKER) run -it -p 80:80 $(IMAGE_TAG)
