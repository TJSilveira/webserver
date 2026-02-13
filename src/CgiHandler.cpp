/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amoiseik <amoiseik@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 15:26:06 by amoiseik          #+#    #+#             */
/*   Updated: 2026/02/13 19:09:56 by amoiseik         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include <sys/wait.h>
#include <cstdlib>
#include <sstream>
#include "Connection.hpp"
#include "Location.hpp"
#include "HttpTransaction.hpp"
#include <arpa/inet.h>

std::map<std::string, std::string>	CgiHandler::_buildEnvMap(const Connection& conn, const Location& loc) {
	std::map<std::string, std::string> env;
	HttpRequest& req = conn.current_transaction->request;

	// 1. Basic variable
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_PROTOCOL"] = req.protocol.empty() ? "HTTP/1.1" : req.protocol;
	env["SERVER_SOFTWARE"] = "Webserv/1.0";
	
	// 2. Server data 
	env["SERVER_NAME"] = conn.server_config->server_name;
	std::stringstream ss_port;
	ss_port << conn.server_config->listen;
	env["SERVER_PORT"] = ss_port.str();

	// 3. Dividing URI on PATH_INFO and QUERY_STRING
	std::string full_uri = req.uri;
	size_t query_pos = full_uri.find('?');
	
	if (query_pos != std::string::npos) {
		env["PATH_INFO"] = full_uri.substr(0, query_pos);
		env["QUERY_STRING"] = full_uri.substr(query_pos + 1);
	} else {
		env["PATH_INFO"] = full_uri;
		env["QUERY_STRING"] = "";
	}

	// 4. Method and headers
	env["REQUEST_METHOD"] = req.method;
	
	std::map<std::string, std::string>::const_iterator it;
	
	it = req.headers.find("Content-Length");
	env["CONTENT_LENGTH"] = (it != req.headers.end()) ? it->second : "0";
	
	it = req.headers.find("Content-Type");
	env["CONTENT_TYPE"] = (it != req.headers.end()) ? it->second : "";

	// 5. Pathes
	std::string root = loc.root.empty() ? conn.server_config->root : loc.root;
	env["DOCUMENT_ROOT"] = root;
	env["PATH_TRANSLATED"] = root + env["PATH_INFO"];
	env["SCRIPT_NAME"] = env["PATH_INFO"];
	
	//5. Cookie
	it = req.headers.find("Cookie");
	if (it != req.headers.end()) {
		env["HTTP_COOKIE"] = it->second;
	}
	
	// 6. Client
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	std::string remote_addr = "127.0.0.1"; // default

	if (getpeername(conn.socket_fd, (struct sockaddr *)&addr, &addr_len) == 0) {
		remote_addr = inet_ntoa(addr.sin_addr);
	}
	env["REMOTE_ADDR"] = remote_addr;

	return env;
}

char** CgiHandler::_prepareEnv(const std::map<std::string, std::string>& envMap) {
	char** envp = NULL;
	try {
		envp = new char*[envMap.size() + 1];
		for (size_t i = 0; i < envMap.size() + 1; ++i) 
			envp[i] = NULL;

		int j = 0;
		std::map<std::string, std::string>::const_iterator it;
		for (it = envMap.begin(); it != envMap.end(); ++it) {
			
			std::string envEntry = it->first + "=" + it->second;
			envp[j] = new char[envEntry.size() + 1];
			std::copy(envEntry.begin(), envEntry.end(), envp[j]);
			envp[j][envEntry.size()] = '\0';
			j++;
		}
	} catch (const std::exception& e) {
		_freeEnv(envp);
		throw;
	}
	return envp;
}

void	CgiHandler::_freeEnv(char** envp) {
	if (!envp) 
		return;
	for (int i = 0; envp[i] != NULL; i++) {
		delete[] envp[i];
	}
	delete[] envp;
}

CgiHandler::CgiHandler() {}
CgiHandler::~CgiHandler() {}

CgiInfo	CgiHandler::execute(const std::string& interpreterPath, 
							const std::string& scriptPath, 
							const std::string& bodyFilePath, 
							char** envp) {
	CgiInfo	info;
	int		pipe_out[2];

	if (pipe(pipe_out) == -1) 
		return info;

	// Set O_NONBLOCK for reading script's result
	if (fcntl(pipe_out[0], F_SETFL, O_NONBLOCK) == -1) {
		close(pipe_out[0]);
		close(pipe_out[1]);
		_freeEnv(envp);
		return info;
	}

	info.pid = fork();
	if (info.pid == -1) {
		close(pipe_out[0]);
		close(pipe_out[1]);
		_freeEnv(envp);
		return info;
	}
	//CHILD PROCESS
	if (info.pid == 0) {
		
		// 1. Read body form file
		if (!bodyFilePath.empty()) {
			int fd_in = open(bodyFilePath.c_str(), O_RDONLY);
			if (fd_in != -1) {
				dup2(fd_in, STDIN_FILENO);
				close(fd_in);
			} else { 
				exit(1);
			}
		} else {
			int dev_null = open("/dev/null", O_RDONLY);
			if (dev_null != -1) {
				dup2(dev_null, STDIN_FILENO);
				close(dev_null);
			}
		}

		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);

		// 3. Execute script
		char* argv[3];
		argv[0] = const_cast<char*>(interpreterPath.c_str());
		argv[1] = const_cast<char*>(scriptPath.c_str());
		argv[2] = NULL;

		execve(argv[0], argv, envp);
		exit(1);
	}

	//PARENT PROCESS
	close(pipe_out[1]);
	_freeEnv(envp);

	info.pipe_fd = pipe_out[0]; // need to add this fd to the poll loop
	info.is_started = true;

	return info;
}


//Change in the pool loop

// Add info.pipe_fd в pollfd.

// In the loop:

// if (fds[i].revents & POLLIN) {
//     char buffer[1024];
//     ssize_t bytes = read(fds[i].fd, buffer, 1024);
//     if (bytes > 0) {
//         client.cgi_response.append(buffer, bytes);
//     } else {
//         // Конец данных
//         close(fds[i].fd);
//         waitpid(info.pid, &status, WNOHANG);
//     }
// }