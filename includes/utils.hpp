/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 17:29:14 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/09 17:55:49 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <limits>
#include <sstream>
#include <cerrno>
#include <cstdlib>
#include "../includes/ConfigError.hpp"

int	extract_and_validate_str_to_int(std::string str);

template<typename T>
std::string ft_int_to_string(T value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

#endif