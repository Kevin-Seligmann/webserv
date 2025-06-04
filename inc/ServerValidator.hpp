/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerValidator.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 18:57:14 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/04 19:56:04 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERVALIDATOR_HPP
# define SERVERVALIDATOR_HPP

# pragma once

# include <iostream>
# include <map>
# include <set>
# include <string>
# include <vector>

# include "Parsed.hpp"
# include "Utils.hpp"

struct ListenKey
{
	std::string	host;
	int			port;

	bool operator<(const ListenKey& other)  const
	{
		if (host != other.host)
			return (host < other.host);

		return (port < other.port);
	}
};

class ServerValidator
{
	public:
		static void validate(const std::vector<ParsedServer>& servers);
};

#endif
