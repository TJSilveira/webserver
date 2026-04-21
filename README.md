*This project has been created as part of the 42 curriculum by tsilveir, amoiseik.*

# Webserv

## Description

Webserv is an HTTP/1.1 server written from scratch in C++98. The goal of the project is to gain a deep understanding of the HTTP protocol by implementing a web server capable of serving static websites, handling file uploads, executing CGI scripts, and being reached from a real web browser.

The server is driven by a single event loop (non-blocking I/O multiplexed through one `epoll` instance) that manages every socket — listening sockets, client connections, and CGI pipes. Its behaviour is fully described through an NGINX-inspired configuration file, which lets the user declare multiple virtual servers, each listening on one or more `host:port` pairs and exposing any number of routes with their own rules (accepted methods, root directory, index file, autoindex, redirections, upload directory, CGI extensions, etc.).

Key features:

- Non-blocking single-`epoll` event loop for sockets and CGI pipes
- Multiple servers listening on multiple `interface:port` pairs
- Multiple virtual hosts sharing the same port (resolved via the `Host` header)
- `GET`, `POST`, and `DELETE` methods
- Static file serving with configurable root, index file, and optional directory listing
- File uploads via multipart/form-data
- HTTP redirections
- Per-route method restrictions and client body size limits
- Configurable custom error pages, with sane defaults when none are provided
- Python CGI support with chunked-request un-chunking and EOF-based response termination
- CGI timeout handling and zombie-process prevention

## Instructions

### Requirements

- A C++ compiler supporting the C++98 standard (`c++` / `g++` / `clang++`)
- `make`
- **Linux** — the server uses the `epoll` API, which is Linux-specific and not available on macOS.

### Build

```sh
make           # build ./webserv
make clean     # remove object files
make fclean    # remove object files and the binary
make re        # fclean + build
```

The binary is compiled with `-Wall -Wextra -Werror -std=c++98`.

### Run

```sh
./webserv <configuration file>
```

A configuration file must be provided as the only argument. A ready-to-use default is shipped in `config/default.conf`:

```sh
./webserv config/default.conf
```

Once running, open your browser (tested with **Brave**) and visit the host and port declared in the configuration file, e.g. `http://localhost:8080/`.

### Configuration file

Configuration files use an NGINX-like syntax. Example servers, locations, CGI setup, and upload directories can be found in:

- `config/default.conf` — suggested starting configuration
- `config/valid/` — additional working examples used to demonstrate features
- `config/invalid/` — malformed configurations used to exercise the parser's error paths
- `config/tester.conf` — configuration used with the provided `tester` binary

Each server block may define:

- `listen` — one or more `host:port` pairs
- `server_name` — virtual host names
- `client_max_body_size` — maximum request body size
- `error_page` — custom error pages
- one or more `location` blocks with: `root`, `index`, `autoindex`, `allow_methods`, `return` (redirect), `upload_store`, and CGI configuration (extension → interpreter).

## Resources

Classic references used during the project:

- **RFC 7230** — HTTP/1.1 Message Syntax and Routing
- **RFC 7231** — HTTP/1.1 Semantics and Content
- **RFC 3875** — The Common Gateway Interface (CGI) Version 1.1
- **RFC 1945** — HTTP/1.0 (suggested as a reference point by the subject)
- [NGINX documentation](https://nginx.org/en/docs/) — reference point for configuration syntax and behaviour comparison
- [MDN Web Docs — HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP) — practical overview of headers, methods, and status codes
- `epoll(7)`, `socket(7)`, `poll(2)`, `fcntl(2)`, `waitpid(2)` man pages
- Beej's Guide to Network Programming — introduction to the BSD sockets API

### Use of AI

AI tools were used throughout the project as a learning and productivity aid. Concretely:

- **Conceptual understanding** — to deepen our understanding of theoretical topics such as the BSD sockets API, the HTTP protocol and its message framing, the `epoll` readiness model, and the CGI/1.1 interface.
- **Feature discussion** — as a sparring partner when designing and implementing nearly every feature of the server (configuration parser, request parser, response builder, connection state machine, CGI handling, virtual host resolution, etc.), to evaluate approaches and edge cases before writing the code ourselves.
- **Testing** — to generate testing scripts (in Python and shell) that stress the server, exercise edge cases in HTTP parsing, and compare behaviour against NGINX.
- **Documentation** — to help write and structure this `README.md`.

All AI-generated suggestions were reviewed, adapted, and validated by us before being integrated into the project, so that every line of code committed is one we understand and can defend.
