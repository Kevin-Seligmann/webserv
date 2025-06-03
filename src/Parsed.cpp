/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsed.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:58:20 by irozhkov          #+#    #+#             */
/*   Updated: 2025/05/30 19:06:19 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parsed.hpp"
#include "Utils.hpp"
#include "ServerValidator.hpp"
#include "ServersManager.hpp"

ParsedServer::ParsedServer(
    const std::string& host,
    int port,
    const std::string& serverName,
    const std::string& root,
    const std::vector<std::string>& indexFiles,
    const std::map<int, std::string>& errorPages,
    const std::map<std::string, ParsedLocations>& locations
)
  : host(host)
  , port(port)
  , server_name(serverName)
  , root(root)
  , index_files(indexFiles)
  , error_pages(errorPages)
  , locations(locations)
{}

ParsedServer::~ParsedServer() {}

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
		else if (key == "allow_methods") {
			while (tokens[i] != ";") loc.allow_methods.push_back(tokens[i++]);
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

Listen parse_listen(const std::vector<std::string>& tokens)
{
	Listen ld;

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
				ld.host = token.substr(0, colon);
				ld.port = to_int(token.substr(colon + 1));
			} 
			else if (std::isdigit(token[0]))
			{
				ld.port = to_int(token);
			}
			else
			{
				ld.host = token;
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
			Listen ld = parse_listen(listen_tokens);
			server.listens.push_back(ld);
		}
		else if (key == "server_name")
		{
			while (tokens[i] != ";") server.server_names.push_back(tokens[i++]);
		}
		else if (key == "root")
			server.root = tokens[i++];
		else if (key == "index")
		{
			while (tokens[i] != ";")
				server.index_files.push_back(tokens[i++]);
		}
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

// ==================== PROCESS FUNCTIONS ====================

// SIMPLE ARBOL DE CONDICIONES QUE EVALUA ARGUMENTOS, PARA SACARLO DEL FLUJO DEL MAIN
int parseProcess(int argc, char **argv, ParsedServers& parsedConfig) {
    // Validate argument count first
    if (argc > 2) {
        std::cerr << std::endl << RED << "ERROR: " << RESET <<
        "This program start with arguments ---> " << YELLOW <<
        "./webserver [filename]" << RESET << std::endl << std::endl;
        return (1);
    }

    try {
        std::string configFile;
        std::string successMessage;
        
        if (argc == 2) {
            // Parse custom config file
            configFile = argv[1];
            successMessage = "Success: starting server with custom config";
        } else {
            // Use default config
            configFile = "conf/default.conf";
            successMessage = "Success: starting server with default config";
        }
        
        // Open and read config file
        std::ifstream file(configFile.c_str());
        if (!file.is_open()) {
            std::cerr << RED << "ERROR: " << RESET <<
            "Cannot open config file: " << configFile << std::endl;
            return (1);
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        file.close();
        
        // Parse configuration
        std::vector<std::string> tokens = tokenize(content);
        parsedConfig = parseConfig(tokens);
        
        // Validate parsed configuration
        ServerValidator::validate(parsedConfig);
        
        std::cout << successMessage << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << RED << "PARSING ERROR: " << RESET << e.what() << std::endl;
        return (1);
    }
    
    return (0);
}


