/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amoiseik <amoiseik@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 15:26:21 by amoiseik          #+#    #+#             */
/*   Updated: 2026/03/02 17:13:51 by amoiseik         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <fcntl.h>
# include <iostream>
# include <map>
# include <string>
# include <unistd.h>
# include <sys/time.h>

class		HttpTransaction;

// Structure for returning data back to main loop (poll)
struct		CgiInfo
{
	int		pipe_fd;
	pid_t	pid;
	bool	is_started;
	std::string	buffer;
	std::string	input_doc_path;
	struct timeval start_time;

	CgiInfo(): pipe_fd(-1), pid(-1), is_started(false){
		start_time.tv_sec = 0;
		start_time.tv_usec = 0;
	}
};

class CgiHandler
{
  private:
	std::map<std::string, std::string> _buildEnvMap(const HttpTransaction &tran,
			int curr_socket);
	char **_prepareEnv(const std::map<std::string, std::string> &envMap);
	void _freeEnv(char **envp);

  public:
	CgiHandler();
	~CgiHandler();

	struct CgiInfo execute(const std::string &interpreterPath,
							const std::string &scriptPath,
							const std::string &bodyFilePath,
							const HttpTransaction &tran,
							int curr_socket);
};

#endif