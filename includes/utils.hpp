/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 17:29:14 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/25 15:16:45 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include "../includes/ConfigError.hpp"
# include <cerrno>
# include <cstdlib>
# include <fstream>
# include <iostream>
# include <limits>
# include <sstream>
# include <string>
# include <algorithm>
# include <vector>

typedef enum s_logger_enum
{
	ERROR,
	INFO,
	OPEN_SOCKET,
	CLOSE_SOCKET,
	WARNING
} t_logger_enum;

/* Extract numbers from strings */
int							extract_and_validate_str_to_int(std::string str);
int							extract_hexa_to_int(std::string str);

/* Open files */
void						open_and_validate_file(char *filename, std::ifstream &file_stream);
bool						open_file(char *filename, std::ifstream &file_stream);

/* String manipulation */
std::string 				file_to_string(std::ifstream &file);
std::vector<std::string>	split_string(const std::string &str, char delim);
std::string					ft_to_upper(const std::string &orginal);
bool						ft_ends_with(const std::string &str, const std::string &to_search);

void						logger(t_logger_enum status, std::string msg, std::ostream &os);

template <typename T>
std::string ft_int_to_string(T value)
{
  std::stringstream ss;
	ss << value;
	return (ss.str());
}

#endif
