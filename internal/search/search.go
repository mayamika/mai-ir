package search

import (
	"context"

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

	var items []*domain.SearchItem
	for _, i := range qr.Items {
		items = append(items, (*domain.SearchItem)(i))
	}

	return &domain.SearchResponse{
		Count: qr.Count,
		Items: items,
	}, nil
}
