package handlers

import (
	"net/http"

	"github.com/go-chi/render"
)

var _ render.Renderer = &errorResponse{}

type errorResponse struct {
	Code   int    `json:"code,omitempty"`
	Reason string `json:"reason,omitempty"`
}

func (e *errorResponse) Render(w http.ResponseWriter, r *http.Request) error {
	render.Status(r, e.Code)
	return nil
}

func badRequestError(reason string) *errorResponse {
	return &errorResponse{http.StatusBadRequest, reason}
}

func internalError(reason string) *errorResponse {
	return &errorResponse{http.StatusInternalServerError, reason}
}
