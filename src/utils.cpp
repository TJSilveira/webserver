/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 17:29:14 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/07 18:13:35 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/utils.hpp"

int	extract_and_validate_str_to_int(std::string str)
{
	long int	num;
	char		*endptr;

	if (str[0] == '-')
	{
		throw ConfigError("Negative numbers are not allowed for listen directive", str);
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
	else if(*endptr != 0)
	{
		throw ConfigError("Invalid int", str);
	}
	return (static_cast<int> (num));
}

