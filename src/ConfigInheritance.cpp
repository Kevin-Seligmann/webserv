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
			if (server.autoindex == AINDX_SERV_ON)
			{
				it->second.setAutoindex(AINDX_LOC_ON);
			}
			else if (server.autoindex == AINDX_SERV_OFF)
			{
				it->second.setAutoindex(AINDX_LOC_OFF);
			}
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

void applyIndexFiles(ParsedServer& server) {

	// Si server no tiene index files pone los defaults
	if (server.index_files.empty())
	{
		server.index_files.push_back("index.html");
		server.index_files.push_back("index.htm");
	}

	// Propagar a locations sin index propio
	std::map<std::string, Location>::iterator it = server.locations.begin();
	for (; it != server.locations.end(); ++it)
	{
		if (it->second.getIndex().empty())
		{
			it->second.setIndex(server.index_files);
		}
	}
}

