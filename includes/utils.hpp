/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 17:29:14 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/14 09:31:13 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <limits>
#include <sstream>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "../includes/ConfigError.hpp"

int	extract_and_validate_str_to_int(std::string str);
int	extract_hexa_to_int(std::string str);
void open_and_validate_file(char *filename, std::ifstream &file_stream);
bool open_file(char *filename, std::ifstream &file_stream);
std::string file_to_string(std::ifstream &file);
bool ft_ends_with(const std::string &str, const std::string &to_search);

template<typename T>
std::string ft_int_to_string(T value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

#endif