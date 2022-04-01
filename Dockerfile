ARG DEBIAN_VERSION=11.3-slim
ARG GOLANG_VERSION=1.18.0-bullseye
ARG NODE_VERSION=17-bullseye-slim

# Build ui.
FROM docker.io/node:${NODE_VERSION} as ui-builder

COPY ui /src
WORKDIR /src

RUN npm install
RUN npm run build

# Build index and cgo libs.
FROM docker.io/debian:${DEBIAN_VERSION} AS cpp-builder

RUN apt-get update && apt-get install -y \
    make \
    g++ \
    zstd

WORKDIR /raw-index
ADD https://dl.vndb.org/dump/vndb-db-latest.tar.zst ./dump.tar.zst
RUN tar -xf dump.tar.zst

WORKDIR /src

COPY . .
RUN make build-cgo
RUN make create-index DB_FILE=/raw-index/db/vn

# Build server.
FROM docker.io/golang:${GOLANG_VERSION} as go-builder

RUN apt-get update && apt-get install -y \
    g++

WORKDIR /src
COPY . .
COPY --from=cpp-builder /src/build/libquery.so /src/build/libquery.so

RUN mkdir -p ./build/
RUN go build -o ./build/ ./...

# Server image.
FROM docker.io/debian:${DEBIAN_VERSION}

WORKDIR /app

COPY --from=cpp-builder /src/build/libquery.so /lib
COPY --from=cpp-builder /src/testdata/index ./index
COPY --from=ui-builder /src/build ./dist
COPY --from=go-builder /src/build/server .

ENV HTTP_ADDR :80
ENV INDEX_PATH /app/index
ENV DIST_PATH /app/dist

EXPOSE 80
ENTRYPOINT [ "./server" ]
CMD []
