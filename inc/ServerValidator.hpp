/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerValidator.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 18:57:14 by irozhkov          #+#    #+#             */
/*   Updated: 2025/05/30 14:14:36 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERVALIDATOR_HPP
# define SERVERVALIDATOR_HPP

# pragma once

# include <iostream>
# include <map>
# include <string>
# include <vector>

# include "Parsed.hpp"

struct ListenKey
{
	std::string	ip;
	int			port;

	bool operator<(const ListenKey& other)  const
	{
		if (ip != other.ip)
			return (ip < other.ip);

		return (port < other.port);
	}
};

class ServerValidator
{
	public:
		static void validate(const std::vector<ParsedServer>& servers);
};

#endif
