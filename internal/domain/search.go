package domain

import (
	"errors"
	"net/http"

	"github.com/go-chi/render"
)

type SearchRequest struct {
	Query string
}

func (sr *SearchRequest) Bind(r *http.Request) error {
	v := r.URL.Query()

	if sr.Query = v.Get("query"); sr.Query == "" {
		return errors.New("query is empty")
	}
	return nil
}

type SearchResponse struct{}

func (sr *SearchResponse) Render(w http.ResponseWriter, r *http.Request) error {
	render.Status(r, http.StatusOK)
	return nil
}
