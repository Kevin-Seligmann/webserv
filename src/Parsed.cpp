/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsed.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:58:20 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/03 13:25:22 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parsed.hpp"

std::vector<std::string> tokenize(const std::string& content)
{
	std::vector<std::string> tokens;
	std::string current;
	bool is_comment = false;

	for (size_t i = 0; i < content.length(); ++i)
	{
		char c = content[i];

		if (c == '#')
		{
			is_comment = true;
			continue;
		}

		if (is_comment)
		{
			if (c == '\n')
			{
				is_comment = false;
			}
			continue;
		}

		if (isspace(c))
		{
			if (!current.empty())
			{
				tokens.push_back(current);
				current.clear();
			}
		}
		else if (c == '{' || c == '}' || c == ';')
		{
			if (!current.empty())
			{
				tokens.push_back(current);
				current.clear();
			}
			tokens.push_back(std::string(1, c));
		}
		else
		{
			current += c;
		}
	}
	if (!current.empty())
	{
		tokens.push_back(current);
	}
	return (tokens);
}

size_t expect(const std::vector<std::string>& tokens, size_t i, const std::string& expected)
{
	if (tokens[i] != expected)
	{
		throw std::runtime_error("Expected: " + expected + ", got: " + tokens[i]);
	}

	return (i + 1);
}

ParsedLocations parseLocation(const std::vector<std::string> &tokens, size_t &i)
{
	ParsedLocations loc;
	loc.path = tokens[i++];
	i = expect(tokens, i, "{");
	while (tokens[i] != "}")
	{
		std::string key = tokens[i++];
		if (key == "root") loc.root = tokens[i++];
		else if (key == "index") loc.index = tokens[i++];
		else if (key == "autoindex") loc.autoindex = (tokens[i++] == "true");
		else if (key == "allow_methods")
		{
			while (tokens[i] != ";")
				loc.allow_methods.push_back(tokens[i++]);
		}
		else if (key == "return") loc.return_path = tokens[i++];
		else if (key == "allow_upload") loc.allow_upload = (tokens[i++] == "true");
		else if (key == "upload_dir") loc.upload_dir = tokens[i++];
		else if (key == "cgi")
		{
			std::string ext = tokens[i++];
			std::string path = tokens[i++];
			loc.cgi[ext] = path;
		}
		if (tokens[i] == ";") ++i;
	}
	++i;
	return (loc);
}

int to_int(const std::string& s)
{
	std::stringstream ss(s);
	int result;
	ss >> result;

	return (result);
}

ParsedListen parse_listen(const std::vector<std::string>& tokens)
{
	ParsedListen ld;

	for (size_t i = 0; i < tokens.size(); ++i)
	{
		const std::string& token = tokens[i];
		if (token == "default_server")
		{
			ld.is_default = true;
		}
		else
		{
			size_t colon = token.find(':');
			if (colon != std::string::npos)
			{
				ld.ip = token.substr(0, colon);
				ld.port = to_int(token.substr(colon + 1));
			} 
			else if (std::isdigit(token[0]))
			{
				ld.port = to_int(token);
			}
			else
			{
				ld.ip = token;
			}
		}
	}

	return (ld);
}

ParsedServer parseServer(const std::vector<std::string> &tokens, size_t &i)
{
	ParsedServer server;

	i = expect(tokens, i, "server");
	i = expect(tokens, i, "{");
	while (tokens[i] != "}")
	{
		std::string key = tokens[i++];
		if (key == "listen")
		{
			std::vector<std::string> listen_tokens;
			while (tokens[i] != ";")
			{
				listen_tokens.push_back(tokens[i++]);
			}
			++i;
			ParsedListen ld = parse_listen(listen_tokens);
			server.listens.push_back(ld);
		}
		else if (key == "server_name")
		{
			while (tokens[i] != ";") server.server_names.push_back(tokens[i++]);
		}
		else if (key == "root")
			server.root = tokens[i++];
		else if (key == "error_page")
		{
			int code = to_int(tokens[i++]);
			while (isdigit(tokens[i][0]))
				code = to_int(tokens[i++]);
			server.error_pages[code] = tokens[i++];
		}
		else if (key == "allow_methods")
		{
			while (tokens[i] != ";")
				server.allow_methods.push_back(tokens[i++]);
        }
		else if (key == "autoindex")
			server.autoindex = (tokens[i++] == "true");
		else if (key == "client_max_body_size")
			server.client_max_body_size = tokens[i++];
		else if (key == "location")
		{
			ParsedLocations loc = parseLocation(tokens, i);
			server.locations[loc.path] = loc;
		}
		if (tokens[i] == ";") ++i;
	}
	++i;

	return (server);
}

std::vector<ParsedServer> parseConfig(const std::vector<std::string> &tokens)
{
	std::vector<ParsedServer> servers;

	size_t i = 0;
	while (i < tokens.size())
	{
		if (tokens[i] == "server")
		{
			servers.push_back(parseServer(tokens, i));
		}
		else
		{
			throw std::runtime_error("Unknown directive outside server block: " + tokens[i]);
		}
	}

	return (servers);
}
