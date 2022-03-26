package search

import (
	"context"

	"github.com/davecgh/go-spew/spew"

	"github.com/mayamika/mai-ir/internal/domain"
	csearch "github.com/mayamika/mai-ir/src/search"
)

type Config struct {
	IndexPath string
}

type Service struct {
	config Config
}

func NewService(c Config) *Service {
	return &Service{
		config: c,
	}
}

func (s *Service) Search(ctx context.Context, r *domain.SearchRequest) (*domain.SearchResponse, error) {
	qr, err := csearch.Search(s.config.IndexPath, r.Query)
	if err != nil {
		return nil, err
	}
	// TODO: Remove debug print.
	n := qr.Count
	if qr.Count > 20 {
		n = 20
	}
	qr.Items = qr.Items[:n]
	spew.Dump(qr)

	return &domain.SearchResponse{}, nil
}
