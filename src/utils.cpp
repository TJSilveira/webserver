/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 17:29:14 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/16 11:32:37 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/utils.hpp"

int	extract_and_validate_str_to_int(std::string str)
{
	long int	num;
	char		*endptr;

	if (str[0] == '-')
	{
		throw ConfigError("Negative numbers are not allowed for listen directive",
				str);
	}
	num = strtol(str.c_str(), &endptr, 10);
	if (num == 0 && str[0] != '0')
	{
		throw ConfigError("Invalid int", str);
	}
	else if (num > static_cast<long int>(std::numeric_limits<int>::max())
			|| errno == ERANGE)
	{
		throw ConfigError("Invalid int", str);
	}
	else if (*endptr != 0)
	{
		throw ConfigError("Invalid int", str);
	}
	return (static_cast<int>(num));
}

int	extract_hexa_to_int(std::string str)
{
	int	n;

	std::istringstream(str) >> std::hex >> n;
	return (n);
}

void	open_and_validate_file(char *filename, std::ifstream &file_stream)
{
	file_stream.open(filename, std::ios::in | std::ios::binary);
	if (file_stream.fail())
	{
		std::cerr << "Error: could not open file." << std::endl;
		exit(EXIT_FAILURE);
	}
}

bool	open_file(char *filename, std::ifstream &file_stream)
{
	file_stream.open(filename, std::ios::in | std::ios::binary);
	if (file_stream.fail())
		return (false);
	return (true);
}

std::string file_to_string(std::ifstream &file)
{
	return (std::string(std::istreambuf_iterator<char>(file),
						std::istreambuf_iterator<char>()));
}

bool	ft_ends_with(const std::string &str, const std::string &to_search)
{
	size_t	len_str;
	size_t	len_to_search;

	len_str = str.size();
	len_to_search = to_search.size();
	if (len_str < len_to_search)
		return (false);
	std::cout << "After here\n";
	if (str.substr(len_str - len_to_search, len_to_search) == to_search)
		return (true);
	else
		return (false);
}
