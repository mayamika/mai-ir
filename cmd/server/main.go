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
	"github.com/mayamika/mai-ir/internal/web"
)

const shutdownTimeout = 10 * time.Second

var (
	httpAddr  string
	indexPath string
	distPath  string
)

func bindEnv() {
	httpAddr = os.Getenv("HTTP_ADDR")
	indexPath = os.Getenv("INDEX_PATH")
	distPath = os.Getenv("DIST_PATH")
}

func newLogger() (*zap.Logger, error) {
	config := zap.NewDevelopmentConfig()
	config.EncoderConfig.EncodeLevel = zapcore.CapitalColorLevelEncoder
	config.Development = false

	return config.Build()
}

func main() {
	bindEnv()

	logger, err := newLogger()
	if err != nil {
		panic(err)
	}

	ss := search.NewService(search.Config{
		IndexPath: indexPath,
	})
	sh := handlers.NewSearch(ss)

	api := chi.NewRouter()
	api.Mount(`/search`, sh.Routes())

	r := chi.NewRouter()
	r.Mount("/", web.Handler(distPath))
	r.Mount("/api", api)

	s := http.Server{
		Addr:    httpAddr,
		Handler: r,
	}

	stop := make(chan os.Signal, 1)
	signal.Notify(stop, os.Interrupt)

	go func() {
		defer close(stop)

		logger.Info("listening http",
			zap.String("addr", s.Addr),
		)
		if err := s.ListenAndServe(); !errors.Is(err, http.ErrServerClosed) {
			logger.Error("http server failed", zap.Error(err))
		}
	}()

	<-stop
	logger.Info("stopping")

	ctx, cancel := context.WithTimeout(context.Background(), shutdownTimeout)
	defer cancel()

	if err := s.Shutdown(ctx); err != nil {
		logger.Error("can't stop http server", zap.Error(err))
	}
}
