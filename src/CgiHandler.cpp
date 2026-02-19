/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 15:26:06 by amoiseik          #+#    #+#             */
/*   Updated: 2026/02/19 14:40:47 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CgiHandler.hpp"
#include "../includes/HttpTransaction.hpp"
#include "../includes/Location.hpp"
#include <arpa/inet.h>
#include <cstdlib>
#include <sstream>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

std::map<std::string, std::string>
CgiHandler::_buildEnvMap(const HttpTransaction &tran, int curr_socket)
{
	size_t				query_pos;
	struct sockaddr_in	addr;
	socklen_t			addr_len;

	std::map<std::string, std::string> env;
	const HttpRequest &req = tran.request;
	// 1. Basic variable
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_PROTOCOL"] = req.protocol.empty() ? "HTTP/1.1" : req.protocol;
	env["SERVER_SOFTWARE"] = "Webserv/1.0";
	// 2. Server data
	// env["SERVER_NAME"] = tran.vir_server->server_name;
	env["SERVER_NAME"] = "localhost";
	std::stringstream ss_port;
	ss_port << tran.vir_server->listen;
	env["SERVER_PORT"] = ss_port.str();
	// 3. Dividing URI on PATH_INFO and QUERY_STRING
	std::string full_uri = req.uri;
	query_pos = full_uri.find('?');
	if (query_pos != std::string::npos)
	{
		env["PATH_INFO"] = full_uri.substr(0, query_pos);
		env["QUERY_STRING"] = full_uri.substr(query_pos + 1);
	}
	else
	{
		env["PATH_INFO"] = full_uri;
		env["QUERY_STRING"] = "";
	}
    env["REQUEST_URI"] = req.uri;

	// 4. Method and headers
	env["REQUEST_METHOD"] = req.method;
	std::map<std::string, std::string>::const_iterator it;
	it = req.headers.find("Content-Length");
	env["CONTENT_LENGTH"] = ft_int_to_string(req.body.length());
	it = req.headers.find("Content-Type");
	env["CONTENT_TYPE"] = (it != req.headers.end()) ? it->second : "";
	// 5. Pathes
	const std::string root = tran.location->root.empty() ? tran.vir_server->root : tran.location->root;
	env["DOCUMENT_ROOT"] = root;
	env["PATH_TRANSLATED"] = env["PATH_INFO"];
	env["SCRIPT_NAME"] = "";
	// 5. Cookie
	it = req.headers.find("Cookie");
	if (it != req.headers.end())
	{
		env["HTTP_COOKIE"] = it->second;
	}
	// 6. Client
	addr_len = sizeof(addr);
	std::string remote_addr = "127.0.0.1"; // default
	if (getpeername(curr_socket, (struct sockaddr *)&addr, &addr_len) == 0)
	{
		remote_addr = inet_ntoa(addr.sin_addr);
	}
	env["REMOTE_ADDR"] = remote_addr;

	// Add remaining headers from request to the CGI headers
	std::map<std::string, std::string>::const_iterator	it_headers = req.headers.begin();
	for (; it_headers != req.headers.end(); it_headers++)
	{
		if (it_headers->first != "Content-Type" && it_headers->first != "Content-Length")
		{
			std::string new_env = "HTTP_"; 
			new_env += it_headers->first;
			new_env = ft_to_upper(new_env);
			std::replace(new_env.begin(), new_env.end(), '-', '_');
			std::replace(new_env.begin(), new_env.end(), '/', '_');
			std::replace(new_env.begin(), new_env.end(), '.', '_');
			env[new_env] = it_headers->second;
		}
	}
	return (env);
}

char **
CgiHandler::_prepareEnv(const std::map<std::string, std::string> &envMap)
{
	char	**envp;
	int		j;

	envp = NULL;
	try
	{
		envp = new char *[envMap.size() + 1];
		for (size_t i = 0; i < envMap.size() + 1; ++i)
			envp[i] = NULL;
		j = 0;
		std::map<std::string, std::string>::const_iterator it;
		for (it = envMap.begin(); it != envMap.end(); ++it)
		{
			std::string envEntry = it->first + "=" + it->second;
			envp[j] = new char[envEntry.size() + 1];
			std::copy(envEntry.begin(), envEntry.end(), envp[j]);
			envp[j][envEntry.size()] = '\0';
			j++;
		}
	}
	catch (const std::exception &e)
	{
		_freeEnv(envp);
		throw;
	}
	return (envp);
}

void CgiHandler::_freeEnv(char **envp)
{
	if (!envp)
		return ;
	for (int i = 0; envp[i] != NULL; i++)
	{
		delete[] envp[i];
	}
	delete[] envp;
}

CgiHandler::CgiHandler(){}

CgiHandler::~CgiHandler(){}

struct CgiInfo CgiHandler::execute(const std::string &interpreterPath,
									const std::string &scriptPath,
									const std::string &bodyFilePath,
									const HttpTransaction &tran,
									int curr_socket)
{
	CgiInfo	info;
	int		pipe_out[2];
	char	**envp;
	int		fd_in;
	int		dev_null;
	char	*argv[3];

	std::map<std::string, std::string> envp_map = _buildEnvMap(tran,
			curr_socket);
	envp = _prepareEnv(envp_map);
	if (pipe(pipe_out) == -1)
		return (info);
	// Set O_NONBLOCK for reading script's result
	if (fcntl(pipe_out[READ], F_SETFL, O_NONBLOCK) == -1)
	{
		close(pipe_out[READ]);
		close(pipe_out[WRITE]);
		_freeEnv(envp);
		return (info);
	}
	info.pid = fork();
	if (info.pid == -1)
	{
		close(pipe_out[READ]);
		close(pipe_out[WRITE]);
		_freeEnv(envp);
		return (info);
	}
	// CHILD PROCESS
	if (info.pid == 0)
	{
		// 1. Read body from file
		if (!bodyFilePath.empty())
		{
			fd_in = open(bodyFilePath.c_str(), O_RDONLY);
			if (fd_in != -1)
			{
				dup2(fd_in, STDIN_FILENO);
				close(fd_in);
			}
			else
			{
				exit(1);
			}
		}
		else
		{
			dev_null = open("/dev/null", O_RDONLY);
			if (dev_null != -1)
			{
				dup2(dev_null, STDIN_FILENO);
				close(dev_null);
			}
		}
		dup2(pipe_out[WRITE], STDOUT_FILENO);

		close(pipe_out[READ]);
		close(pipe_out[WRITE]);
		// 3. Execute script
		argv[0] = const_cast<char *>(interpreterPath.c_str());
		argv[1] = const_cast<char *>(scriptPath.c_str());
		argv[2] = NULL;
		execve(argv[0], argv, envp);
		write(STDERR_FILENO, "execve failed\n", 14);
		exit(1);
	}
	// PARENT PROCESS
	close(pipe_out[WRITE]);
	_freeEnv(envp);
	info.pipe_fd = pipe_out[READ];
	info.is_started = true;
	info.input_doc_path = bodyFilePath;
	return (info);
}
