/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigError.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:17:48 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/16 13:01:45 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGERROR_HPP
# define CONFIGERROR_HPP

# include <sstream>
# include <string>

class ConfigError : public std::exception
{
	std::string msg;

	public:
	ConfigError(const std::string &message, const std::string problematic_token)
	{
		std::stringstream ss;

		ss << "Config Error: " << message << ". Problematic token: '"
			<< problematic_token << "'\n";
		msg = ss.str();
	}

	virtual const char *what() const throw()
	{
		return (msg.c_str());
	}

	virtual ~ConfigError() throw(){};
};

#endif
