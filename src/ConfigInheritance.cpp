/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigInheritance.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/16 16:41:04 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/16 20:18:04 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigInheritance.hpp"

void applyAutoindex(ParsedServer& server)
{
	for (std::map<std::string, Location>::iterator it = server.locations.begin();
		 it != server.locations.end(); ++it)
	{
		if (it->second.getAutoindex() == AINDX_DEF_OFF)
		{
			it->second.setAutoindex(server.autoindex);
		}
	}
}

void applyAllowMethods(ParsedServer& server)
{
	if (server.allow_methods.empty())
		server.allow_methods.push_back("GET");

	for (std::map<std::string, Location>::iterator it = server.locations.begin();
         it != server.locations.end(); ++it)
	{
		std::vector<std::string> methods = it->second.getMethods();

		if (methods.empty())
		{
			it->second.setMethods(server.allow_methods);
		}
	}
}

