package web

import (
	"errors"
	"io/fs"
	"net/http"
)

type fileSystem struct {
	http.FileSystem
}

func (fsys *fileSystem) Open(name string) (http.File, error) {
	f, err := fsys.FileSystem.Open(name)
	if errors.Is(err, fs.ErrNotExist) {
		return fsys.FileSystem.Open("index.html")
	}
	return f, err
}

// Handler provides http.Handler for serving ui dist.
func Handler(distPath string) http.Handler {
	fsys := &fileSystem{http.Dir(distPath)}
	return http.FileServer(fsys)
}
