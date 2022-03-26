package main

import (
	"context"
	"errors"
	"net/http"
	"os"
	"os/signal"
	"time"

	"github.com/go-chi/chi/v5"
	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"

	"github.com/mayamika/mai-ir/internal/handlers"
	"github.com/mayamika/mai-ir/internal/search"
)

const shutdownTimeout = 10 * time.Second

func main() {
	logger, err := newLogger()
	if err != nil {
		panic(err)
	}

	sc := search.Config{
		IndexPath: "testdata/index",
	}

	ss := search.NewService(sc)
	sh := handlers.NewSearch(ss)

	r := chi.NewRouter()
	r.Mount(`/search`, sh.Routes())

	s := http.Server{
		Addr:    ":8080",
		Handler: r,
	}

	go func() {
		logger.Info("listening http",
			zap.String("addr", s.Addr),
		)

		if err := s.ListenAndServe(); !errors.Is(err, http.ErrServerClosed) {
			logger.Error("http server failed", zap.Error(err))
		}
	}()

	signals := make(chan os.Signal, 1)
	signal.Notify(signals, os.Interrupt)

	<-signals
	logger.Info("stopping")

	ctx, cancel := context.WithTimeout(context.Background(), shutdownTimeout)
	defer cancel()

	if err := s.Shutdown(ctx); err != nil {
		logger.Error("can't stop http server", zap.Error(err))
	}
}

func newLogger() (*zap.Logger, error) {
	config := zap.NewDevelopmentConfig()
	config.EncoderConfig.EncodeLevel = zapcore.CapitalColorLevelEncoder
	config.Development = false

	return config.Build()
}
