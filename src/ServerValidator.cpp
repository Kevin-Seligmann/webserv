/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvisca-g <mvisca-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 12:44:48 by irozhkov          #+#    #+#             */
/*   Updated: 2025/10/04 11:34:03 by mvisca-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerValidator.hpp"

// Temporary color macros to fix compilation
#define RED        "\033[1;91m"
#define GREEN      "\033[1;92m"
#define YELLOW     "\033[1;93m"
#define BLUE       "\033[1;94m"
#define RESET      "\033[0m"

void ServerValidator::validate(const std::vector<ParsedServer>& servers)
{
	std::map<ListenKey, std::vector<const ParsedServer*> > listen_map;
	std::map<std::string, std::set<ListenKey> > name_to_hostports;

	for (size_t s = 0; s < servers.size(); ++s)
	{
		const ParsedServer& server = servers[s];
		for (size_t l = 0; l < server.listens.size(); ++l)
		{
			const Listen& ld = server.listens[l];
			ListenKey key;
			key.host = ld.host;
			key.port = ld.port;

			listen_map[key].push_back(&server);

			for (size_t n = 0; n < server.server_names.size(); ++n)
			{
				name_to_hostports[server.server_names[n]].insert(key);
			}
		}

		for (std::map<std::string, Location>::const_iterator loc_it = server.locations.begin(); loc_it != server.locations.end(); loc_it ++)
		{
			if (loc_it->second.getAlias().size() > 0 && loc_it->second.getRoot().size() > 0)
				throw std::runtime_error("Root and alias are incompatible");
		}
	}

	std::map<ListenKey, std::vector<const ParsedServer*> >::iterator it1 = listen_map.begin();
	while (it1 != listen_map.end())
	{
		const ListenKey& key = it1->first;
		const std::vector<const ParsedServer*>& servers_list = it1->second;

		if (servers_list.size() > 1)
		{
			std::stringstream os;
			os << "Multiple servers on " << YELLOW <<
			key.host << ":" << key.port << RESET << ". According to the subject requirements, " <<
			"if two or more servers are configured with the same host:port, " << YELLOW <<
			"the first one in order is considered the primary" << RESET;
			Logger::getInstance().warning(os.str());
		}
		++it1;
	}

	std::map<std::string, std::set<ListenKey> >::iterator it2 = name_to_hostports.begin();
	while (it2 != name_to_hostports.end())
	{
		if (it2->second.size() > 1)
		{
			std::stringstream os;
			os << "Server name " <<  YELLOW << 
			it2->first << RESET << " used on multiple host:port combinations. " <<
 			"According to the subject requirements, if two or more servers are " << 
			"configured with the same host:port, " << YELLOW <<
			"the first one in order is considered the primary" << RESET;
			Logger::getInstance().warning(os.str());
		}
		++it2;
	}
}

