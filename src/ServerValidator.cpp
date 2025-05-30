/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 12:44:48 by irozhkov          #+#    #+#             */
/*   Updated: 2025/05/30 14:14:14 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerValidator.hpp"

void ServerValidator::validate(const std::vector<ParsedServer>& servers)
{
	std::map<ListenKey, std::vector<const ParsedServer*> > listen_map;
    std::map<ListenKey, int> default_server_count;

	for (size_t s = 0; s < servers.size(); ++s) 
	{
		const ParsedServer &server = servers[s];
		for (size_t l = 0; l < server.listens.size(); ++l)
		{
			const ParsedListen &ld = server.listens[l];
			ListenKey key;
			key.ip = ld.ip;
			key.port = ld.port;

			listen_map[key].push_back(&server);
			if (ld.is_default)
				++default_server_count[key];
		}
	}

	std::map<ListenKey, std::vector<const ParsedServer*> >::iterator it;
	for (it = listen_map.begin(); it != listen_map.end(); ++it) 
	{
		const ListenKey &key = it->first;
		const std::vector<const ParsedServer*> &servers_list = it->second;

		bool allUnnamed = true;
		for (size_t i = 0; i < servers_list.size(); ++i)
		{
			if (!servers_list[i]->server_names.empty())
			{
				allUnnamed = false;
				break;
			}
		}
		if (servers_list.size() > 1 && allUnnamed)
		{
			std::cerr << "WARNING: Multiple unnamed servers on "
			<< key.ip << ":" << key.port
			<< " — routing may be ambiguous.\n";
		}

		if (default_server_count[key] > 1) {
			std::cerr << "ERROR: More than one default_server on "
			<< key.ip << ":" << key.port << ".\n";
		}
	}
}
