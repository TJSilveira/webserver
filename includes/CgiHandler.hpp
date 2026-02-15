/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 15:26:21 by amoiseik          #+#    #+#             */
/*   Updated: 2026/02/14 10:17:22 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <iostream>
#include <string>
#include <map>
#include <unistd.h>
#include <fcntl.h>

class HttpTransaction;

// Structure for returning data back to main loop (poll)
struct CgiInfo {
	int			pipe_fd;
	pid_t		pid;
	bool		is_started;
	std::string buffer;
	std::string	input_doc_path;

	CgiInfo() : pipe_fd(-1), pid(-1), is_started(false) {}
};

class CgiHandler {
	private:
		std::map<std::string, std::string>	_buildEnvMap(const HttpTransaction& tran, int curr_socket);
		char**								_prepareEnv(const std::map<std::string, std::string>& envMap);
		void								_freeEnv(char** envp);

	public:
		CgiHandler();
		~CgiHandler();

		struct CgiInfo	execute(const std::string& interpreterPath, 
							const std::string& scriptPath, 
							const std::string& bodyFilePath,
							const HttpTransaction &tran,
							int curr_socket);
};

#endif