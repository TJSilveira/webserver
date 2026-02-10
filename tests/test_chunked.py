
import socket
import time

def send_chunked_request(host, port, name, request_line, headers, chunks):
    print(f"--- Running Test: {name} ---")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))

        # Send Request Line and Headers
        header_raw = request_line + "\r\n"
        for k, v in headers.items():
            header_raw += f"{k}: {v}\r\n"
        header_raw += "\r\n"
        s.sendall(header_raw.encode())
        print("Sent headers...")

        # Send Chunks
        for chunk in chunks:
            if isinstance(chunk, str):
                size = hex(len(chunk))[2:]
                payload = f"{size}\r\n{chunk}\r\n"
            else: # Zero chunk
                payload = "0\r\n\r\n"

            s.sendall(payload.encode())
            print(f"Sent chunk: {payload!r}")
            time.sleep(0.5) # Simulate network delay to test server buffering

        response = s.recv(4096)
        print("Response received:")
        print(response.decode())
        s.close()
    except Exception as e:
        print(f"Error: {e}")
    print("\n")

# 1. Basic POST request with 3 text chunks
send_chunked_request(
    "localhost", 4241, "Basic POST",
    "POST /upload HTTP/1.1",
    {"Host": "localhost:4241", "Transfer-Encoding": "chunked", "Content-Type": "text/plain"},
    ["Hello ", "from ", "Perplexity!", 0]
)

# 2. Request with an empty body (immediate zero-size chunk)
send_chunked_request(
    "localhost", 4241, "Empty Body",
    "POST /empty HTTP/1.1",
    {"Host": "localhost:4241", "Transfer-Encoding": "chunked"},
    [0]
)

# 3. Request with large hex sizes (testing hex parsing)
send_chunked_request(
    "localhost", 4241, "Large Hex Size",
    "POST /large HTTP/1.1",
    {"Host": "localhost:4241", "Transfer-Encoding": "chunked"},
    ["A" * 15, "B" * 32, 0]
)
