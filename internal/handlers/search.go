package handlers

import (
	"context"
	"net/http"

	"github.com/go-chi/chi/v5"
	"github.com/go-chi/render"

	"github.com/mayamika/mai-ir/internal/domain"
)

var (
	errEncodeFailed = badRequestError("can't encode response")
)

type SearchService interface {
	Search(ctx context.Context, r *domain.SearchRequest) (*domain.SearchResponse, error)
}

type Search struct {
	service SearchService
}

func NewSearch(ss SearchService) *Search {
	return &Search{
		service: ss,
	}
}

func (h *Search) Routes() chi.Router {
	r := chi.NewRouter()

	r.Route(`/`, func(r chi.Router) {
		r.Get(`/`, h.get)
	})
	return r
}

func (h *Search) get(w http.ResponseWriter, r *http.Request) {
	var sr domain.SearchRequest
	if err := sr.Bind(r); err != nil {
		_ = render.Render(w, r, badRequestError(err.Error()))
		return
	}

	res, err := h.service.Search(r.Context(), &sr)
	if err != nil {
		_ = render.Render(w, r, internalError(err.Error()))
		return
	}

	if err := render.Render(w, r, res); err != nil {
		_ = render.Render(w, r, errEncodeFailed)
		return
	}
}
