package main

import (
	"fmt"
	"io"
	"net/http"
	"sync"
	"time"
)

func runTest(client *http.Client, workers int, requests int, url string) error {
	var wg sync.WaitGroup
	errors := make(chan error, workers*requests)

	for w := 0; w < workers; w++ {
		wg.Add(1)
		go func(workerID int) {
			defer wg.Done()
			for i := 0; i < requests; i++ {
				resp, err := client.Get(url)
				if err != nil {
					errors <- fmt.Errorf("worker %d req %d: %v", workerID, i, err)
					return
				}
				io.Copy(io.Discard, resp.Body)
				resp.Body.Close()
			}
		}(w)
	}

	wg.Wait()
	close(errors)

	failed := 0
	for err := range errors {
		fmt.Println("ERROR:", err)
		failed++
	}
	if failed == 0 {
		fmt.Printf("OK: %d workers x %d requests on %s\n", workers, requests, url)
		return nil
	}
	return fmt.Errorf("FAILED: %d errors", failed)
}

func main() {
	client := &http.Client{
		Timeout: 10 * time.Second,
		Transport: &http.Transport{
			MaxIdleConnsPerHost: 50,
			DisableKeepAlives:   false,
		},
	}

	base := "http://localhost:4241"

	tests := []struct {
		workers  int
		requests int
		path     string
	}{
		{5, 15, "/"},
		{20, 5000, "/"},
		{128, 50, "/directory/nop"},
	}

	for _, t := range tests {
		fmt.Printf("\nTest multiple workers(%d) doing multiple times(%d): GET on %s\n", t.workers, t.requests, t.path)
		if err := runTest(client, t.workers, t.requests, base+t.path); err != nil {
			fmt.Println("FATAL ERROR ON LAST TEST:", err)
			return
		}
	}

	// Final single request replicating the tester's closing check
	fmt.Println("\nFinal GET on /")
	resp, err := client.Get(base + "/")
	if err != nil {
		fmt.Println("FATAL ERROR ON LAST TEST:", err)
		return
	}
	io.Copy(io.Discard, resp.Body)
	resp.Body.Close()
	fmt.Println("OK: final GET /")
}
