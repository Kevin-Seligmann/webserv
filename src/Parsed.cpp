/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsed.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:58:20 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/16 20:18:01 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigInheritance.hpp"
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
	const std::vector<std::string>& methods,
    const std::map<std::string, Location>& locations
)
  : server_names(serverNames)
  , root(root)
  , index_files(indexFiles)
  , error_pages(errorPages)
  , allow_methods(methods)
  , autoindex(AINDX_DEF_OFF)
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

/*
TODO
En parseLocation
error_page
debería validar que los archivos existan al parsear el path
*/
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
	else if (loc.getMatchType() == Location::UNSET) {
		loc.setMatchType(Location::PREFIX);
		loc.setPath(tokens[i]);
		++i;
	}

	i = expect(tokens, i, "{");
	while (i < tokens.size() && tokens[i] != "}")
	{
		std::string key = tokens[i++];
		if (key == "location")
        {
            throw std::runtime_error("Locations inside location are not supported.");
		}
		else if (key == "allow_methods")
		{
			std::vector<std::string> methods;
			while (i < tokens.size() && tokens[i] != ";")
				methods.push_back(tokens[i++]);
			if (i < tokens.size() && tokens[i] == ";") ++i;
			loc.setMethods(methods);
		}
		else if (key == "root")
		{
			if (i < tokens.size()) loc.setRoot(tokens[i++]);
		}
		else if (key == "index")
		{
			std::vector<std::string> index_vec;
			while (i < tokens.size() && tokens[i] != ";") {
				index_vec.push_back(tokens[i++]);
			}
			if (index_vec.empty()) {
				index_vec.push_back("index.html");
			}
			loc.setIndex(index_vec);
			if (i < tokens.size() && tokens[i] != ";") ++i;
		}
		else if (key == "autoindex")
		{
			std::string autoindex_value = tokens[i++];
			if (autoindex_value == "true" || autoindex_value == "on")
				loc.setAutoindex(AINDX_LOC_ON);
			else if (autoindex_value == "false" || autoindex_value == "off")
				loc.setAutoindex(AINDX_LOC_OFF);
			else {
				loc.setAutoindex(AINDX_LOC_OFF);
				std::cout << "Invalid argument for 'autoindex' directive. Setting default = false." << std::endl;
			}
			if (i < tokens.size() && tokens[i] == ";") ++i;
		}
		else if (key == "redirect") 
		{
            if (i < tokens.size()) loc.setRedirect(tokens[i++]);
        }
        else if (key == "cgi_extension")
		{
            if (i < tokens.size()) loc.setCgixtension(tokens[i++]);
        }
        else if (key == "allow_upload")
		{
            if (i < tokens.size()) {
                std::string value = tokens[i++];
                loc.setAllowUpload(value == "on" || value == "true");
            }
        }
		else if (key == "error_page")
		{
			std::vector<int> codes;
			while (i < tokens.size() && tokens[i] != ";" && isdigit(tokens[i][0]))
				codes.push_back(to_int(tokens[i++]));
			
			if (i < tokens.size() && tokens[i] != ";")
			{
				std::string error_page_path = tokens[i++];
				for (size_t j = 0; j < codes.size(); ++j) {
					loc.setErrorPage(codes[j], error_page_path); 
				}
			}
			if (i < tokens.size() && tokens[i] == ";") ++i;
		}
		else if (key == "body_size") {

			std::string value = tokens[i++];
			size_t limit = str_to_sizet(value, ULONG_MAX);
			loc.setMaxBodySize(limit);
			if (i < tokens.size() && tokens[i] == ";") ++i;
		}
		else if (key == "alias"){
			loc.setAlias(tokens[i++]);
		}
        else
		{
            if (i < tokens.size() && tokens[i] == ";") ++i;
        }
	}
	if (i < tokens.size() && tokens[i] == "}") ++i;


/* TO_DELETE
// debug

	Logger::getInstance().info("=== PARSED LOCATION DEBUG ===");
	Logger::getInstance().info("  path: '" + loc.getPath() + "'");
	Logger::getInstance().info("  match_type: " + std::string(loc.getMatchType() == Location::EXACT ? "EXACT" : "PREFIX"));
	Logger::getInstance().info("  methods count: " + wss::i_to_dec(loc.getMethods().size()));
	for (size_t j = 0; j < loc.getMethods().size(); ++j) {
		Logger::getInstance().info("    method[" + wss::i_to_dec(j) + "]: " + loc.getMethods()[j]);
	}
	Logger::getInstance().info("  root: '" + loc.getRoot() + "'");

	std::ostringstream oss;
	for (size_t i = 0; i < loc.getIndex().size(); ++i) {
		oss << loc.getIndex()[i] << (i + 1 < loc.getIndex().size() ? " " : "");
	}
	Logger::getInstance().info(std::string("  index: '") + oss.str() + "'");
	Logger::getInstance().info("  autoindex: " + wss::i_to_dec(static_cast<int>(loc.getAutoindex())));
	Logger::getInstance().info("  redirect: '" + loc.getRedirect() + "'");
	Logger::getInstance().info("  cgi_extension: '" + loc.getCgiExtension() + "'");
	Logger::getInstance().info("  allow_upload: " + std::string(loc.getAllowUpload() ? "TRUE" : "FALSE"));
	Logger::getInstance().info("=== END PARSED LOCATION DEBUG ===");

//end debug */

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
				if (brace_close == std::string::npos)
					throw std::runtime_error("Unclosed brackets in listen directive");
				throw std::runtime_error("IPv6 addresses are not allowed: " + token);
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
				server.autoindex = AINDX_SERV_ON;
			else if (autoindex_value == "false" || autoindex_value == "off")
				server.autoindex = AINDX_SERV_OFF;
			else {
				server.autoindex = AINDX_DEF_OFF;
				std::cout << "Invalid argument for 'autoindex' directive. Setting default = false." << std::endl;
			}
		}
		else if (key == "client_max_body_size")
			server.client_max_body_size = tokens[i++];
		else if (key == "location")
		{
			Location loc = parseLocation(tokens, i);
			std::string map_key = loc.getPath();
			if (loc.getMatchType() == Location::EXACT) {
				map_key = "=" + map_key;
			}
			server.locations[loc.getPath()] = loc;
		}
		if (i < tokens.size() && tokens[i] == ";") ++i;
	}
	++i;

	applyAutoindex(server);
	applyAllowMethods(server);
	applyIndexFiles(server);

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


		for (size_t i = 0; i < parsedConfig.size(); ++i)
		{
			printServerConfig(parsedConfig[i]);
		}

        ServerValidator::validate(parsedConfig);

        std::cout << BLUE << successMessage << RESET << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << RED << "PARSING ERROR: " << RESET << e.what() << std::endl;
        return (1);
    }
    
    return (0);
}


