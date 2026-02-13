/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amoiseik <amoiseik@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 15:26:21 by amoiseik          #+#    #+#             */
/*   Updated: 2026/02/13 17:01:45 by amoiseik         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <iostream>
#include <string>
#include <map>
#include <unistd.h>
#include <fcntl.h>

// Structure for returning data back to main loop (poll)
struct CgiInfo {
	int		pipe_fd;
	pid_t	pid;
	bool	is_started;

	CgiInfo() : pipe_fd(-1), pid(-1), is_started(false) {}
};

class CgiHandler {
	private:
		std::map<std::string, std::string>	_buildEnvMap(const Connection& conn, const Location& loc);
		char**								_prepareEnv(const std::map<std::string, std::string>& envMap);
		void								_freeEnv(char** envp);

	public:
		CgiHandler();
		~CgiHandler();

		CgiInfo	execute(const std::string& interpreterPath, 
						const std::string& scriptPath, 
						const std::string& bodyFilePath, 
						char** envp);
};

#endif