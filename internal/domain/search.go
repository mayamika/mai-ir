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

type SearchItem struct {
	ID          string `json:"id"`
	Title       string `json:"title"`
	Image       string `json:"image"`
	Description string `json:"description"`
}

type SearchResponse struct {
	Count int           `json:"count"`
	Items []*SearchItem `json:"items"`
}

func (sr *SearchResponse) Render(w http.ResponseWriter, r *http.Request) error {
	render.Status(r, http.StatusOK)
	return nil
}
