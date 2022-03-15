package main

import (
	"embed"
	"flag"
	"io"
	"log"
	"net/http"
	"time"
)

var listenAddr = flag.String("listen", "0.0.0.0:8080", "port to listen on")

// This embeds the entirety of the static directory in the binary.
//go:embed static
var staticContent embed.FS


func handleGenericRequest(rw http.ResponseWriter, r *http.Request) {
	// Actually serve the file.
	// We can't use http.ServeFile because there are no real files - our static content is embedded in the binary.
	f, err := staticContent.Open("static/index.html")
	if err != nil {
		http.NotFound(rw, r)
		return
	}
	defer f.Close()
	rw.Header().Add("Cache-Control", "public, max-age=86400")
	http.ServeContent(rw, r, r.URL.Path, time.Time{}, f.(io.ReadSeeker))
}

func main() {
	flag.Parse()
	sm := http.NewServeMux()
	sm.HandleFunc("/", handleGenericRequest)
	sm.HandleFunc("/healthz", func(w http.ResponseWriter, r *http.Request) {
		_, _ = w.Write([]byte("rebble-emoji"))
	})

	if err := http.ListenAndServe(*listenAddr, sm); err != nil {
		log.Fatalf("Failed to listen and serve: %v.\n", err)
	}
}
