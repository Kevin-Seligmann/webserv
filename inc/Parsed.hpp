/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsed.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:51:01 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/16 18:30:04 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_PARSED_HPP
#define WEBSERV_PARSED_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Enums.hpp"
#include "Listen.hpp"
#include "Location.hpp"

class ServersManager;

struct ParsedServer {
	std::vector<Listen> listens;
	std::vector<std::string> server_names;
	std::string root;
	std::vector<std::string> index_files;
	std::map<int, std::string> error_pages;
	std::vector<std::string> allow_methods;
	AutoIndexState autoindex;
	std::string client_max_body_size;
	std::map<std::string, Location> locations;

	ParsedServer()
		: autoindex(AINDX_DEF_OFF) {}

	ParsedServer(const Listen& listen,
				 const std::vector<std::string>& server_names,
				 const std::string& root,
				 const std::vector<std::string>& index_files,
				 const std::map<int, std::string>& error_pages,
				 const std::vector<std::string>& allow_methods,
				 const std::map<std::string, Location>& locations);

	~ParsedServer();
};

typedef std::vector<ParsedServer> ParsedServers;

// PARSING

std::vector<std::string> tokenize(const std::string& content);

size_t expect(const std::vector<std::string>& tokens, size_t i, const std::string& expected);

int to_int(const std::string& s);

Listen parse_listen(const std::vector<std::string>& tokens);

Location parseLocation(const std::vector<std::string>& tokens, size_t& i);

ParsedServer parseServer(const std::vector<std::string>& tokens, size_t& i);

std::vector<ParsedServer> parseConfig(const std::vector<std::string>& tokens);

// PROCESS
int parseProcess(int argc, char **argv, ParsedServers& parsedConfig);

#endif // WEBSERV_PARSED_HPP
