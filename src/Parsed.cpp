/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsed.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:58:20 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/12 14:17:10 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parsed.hpp"
#include "Utils.hpp"
#include "ServerValidator.hpp"
#include "VirtualServersManager.hpp"

// Temporary color macros to fix compilation
#define RED        "\033[1;91m"
#define GREEN      "\033[1;92m"
#define YELLOW     "\033[1;93m"
#define BLUE       "\033[1;94m"
#define RESET      "\033[0m"

ParsedServer::ParsedServer(
    const Listen& listen,
    const std::vector<std::string>& serverNames,
    const std::string& root,
    const std::vector<std::string>& indexFiles,
    const std::map<int, std::string>& errorPages,
    const std::map<std::string, Location>& locations
)
  : server_names(serverNames)
  , root(root)
  , index_files(indexFiles)
  , error_pages(errorPages)
  , autoindex(false)
  , locations(locations)
{
    listens.push_back(listen);
}

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

Location parseLocation(const std::vector<std::string> &tokens, size_t &i)
{
	Location loc;
	
	if (tokens[i] == "=") {
		loc.setMatchType(Location::EXACT);
		++i;
		loc.setPath(tokens[i]);
		++i;
	}
	else if (tokens[i] == "~" || tokens[i] == "~*") {
		throw std::runtime_error("Invalid regex location");
	}
	else {
		loc.setPath(tokens[i]);
		++i;
	}

	// TODO: Implement proper Location parsing when class has setters
	// For now, just skip to the closing brace
	(void)loc; // Suppress unused variable warning
	
	i = expect(tokens, i, "{");
	while (i < tokens.size() && tokens[i] != "}")
	{
		++i; // Skip all tokens until closing brace
	}
	++i; // Skip the closing brace
	
	return loc;
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
			printParsingMessage(DEFAULT_SERVER);
		}
		else
		{
			size_t brace_open = token.find('[');
			size_t brace_close = token.find(']');
			size_t point = token.find('.');
			size_t colon = token.find(':');
			if (brace_open != std::string::npos) 
			{
				printParsingMessage(IPV6_HOST);
				if (brace_close == std::string::npos) {
					throw std::runtime_error("Unclosed brackets in listen directive"); }
				if (brace_close + 1 < token.size() && token[brace_close + 1] == ':') {
					ld.port = to_int(token.substr(brace_close + 2)); }
			}
			else if (colon != std::string::npos)
			{
				if (token.substr(0, colon) == "localhost")
				{
					printParsingMessage(LOCAL_HOST);
                    ld.host = getLoopbackAddress();
					ld.port = to_int(token.substr(colon + 1));
				}
                else if (token.substr(0, colon).size() == 1 && token.substr(0, colon)[0] == '*')
				{
					printParsingMessage(ASTERIKS_HOST);
					ld.port = to_int(token.substr(colon + 1));
				}
				else 
				{
					ld.host = token.substr(0, colon);
					ld.port = to_int(token.substr(colon + 1));
				}
			} 
			else if (std::isdigit(token[0]) && point == std::string::npos)
			{
				ld.port = to_int(token);
			}
			else
			{
				if (token == "localhost") {
					ld.host = getLoopbackAddress(); }
				else if (token[0] != '*')
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
			std::vector<int>codes;
			while (isdigit(tokens[i][0]))
				codes.push_back(to_int(tokens[i++]));
			
			std::string error_page_path = tokens[i++];
			for (size_t j = 0; j < codes.size(); ++j)
			{
				server.error_pages[codes[j]] = error_page_path;
			}
		}
		else if (key == "allow_methods")
		{
			while (tokens[i] != ";")
				server.allow_methods.push_back(tokens[i++]);
        }
		else if (key == "autoindex")
		{
			std::string autoindex_value = tokens[i++];
			if (autoindex_value == "true" || autoindex_value == "on")
				server.autoindex = true;
			else if (autoindex_value == "false" || autoindex_value == "off")
				server.autoindex = false;
			else {
				server.autoindex = false;
				std::cout << "Invalid argument for 'autoindex' directive. Setting default = false." << std::endl;
			}
		}
		else if (key == "client_max_body_size")
			server.client_max_body_size = tokens[i++];
		else if (key == "location")
		{
			Location loc = parseLocation(tokens, i);
			server.locations[loc.getPath()] = loc;
		}
		if (tokens[i] == ";") ++i;
	}
	++i;

	return (server);
}

/*

Posibles mejoras de robustez de la validacion en server validator
 Códigos de error HTTP válidos (100-599)
 Valores de autoindex válidos (on/off/true/false)
 Tamaños de client_max_body_size válidos
 Métodos HTTP válidos en allow_methods
 Rutas de archivos válidas en root, index, error_page
 Sintaxis de locations válida
 Sintaxis de listen válida (puertos 1-65535)

*/

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

// SIMPLE ARBOL DE CONDICIONES QUE EVALUA ARGUMENTOS, PARA SACARLO DEL FLUJO DEL MAIN
int parseProcess(int argc, char **argv, ParsedServers& parsedConfig) {

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
			if (checkFile(argv[1]) == -1)
				return (1);
            configFile = argv[1];
            successMessage = "Success: starting server with custom config";
        } else {
            configFile = "conf/default.conf";
            successMessage = "Success: starting server with default config";
        }
        
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
        
        std::vector<std::string> tokens = tokenize(content);

		parsedConfig = parseConfig(tokens);
        
        ServerValidator::validate(parsedConfig);
        
        std::cout << BLUE << successMessage << RESET << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << RED << "PARSING ERROR: " << RESET << e.what() << std::endl;
        return (1);
    }
    
    return (0);
}


