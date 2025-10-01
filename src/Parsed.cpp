/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsed.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvisca-g <mvisca-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:58:20 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/30 17:58:37 by mvisca-g         ###   ########.fr       */
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
	tokens.reserve(100);
	std::string current;
	current.reserve(64);
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
	if (i >= tokens.size())
	{
		throw std::runtime_error("Expected: " + expected + ", but reached end of tokens");
	}
	if (tokens[i] != expected)
	{
		throw std::runtime_error("Expected: " + expected + ", got: " + tokens[i]);
	}

	return (i + 1);
}

bool isServerDirective(const std::string& token) {
    return (token == "listen" || token == "server_name" || token == "root" || 
            token == "index" || token == "error_page" || token == "allow_methods" ||
            token == "autoindex" || token == "client_max_body_size" || token == "location");
}

bool isLocationDirective(const std::string& token) {
	return (token == "allow_methods" || token == "root" || token == "index" || 
			token == "autoindex" || token == "redirect" || token == "cgi_extension" ||
			token == "allow_upload" || token == "error_page" || token == "client_max_body_size" ||
			token == "alias");
}

Location parseLocation(const std::vector<std::string> &tokens, size_t &i)
{
	Location loc;
	
	if (i < tokens.size() && (tokens[i] == "~" || tokens[i] == "~*"))
		throw std::runtime_error("Invalid regex location");
	else if (i < tokens.size() && tokens[i] == "=") {
		loc.setMatchType(Location::EXACT);
		++i;
		if (i >= tokens.size()) {
			CODE_ERR("Missing path after '=' in location");
		}
	} else {
		loc.setMatchType(Location::PREFIX);
	}

	if (i >= tokens.size())
		CODE_ERR("Missing location path");
	
	std::string temp_path = tokens[i];
	for (size_t i = 0; i < temp_path.size() - 1; ++i)
	{
		if (temp_path[i] == '/' &&
			temp_path[i] == temp_path[i + 1])
		{
			CODE_ERR("Double '/' in location path");
		}
	}
	
	if (temp_path.size() > 1 && temp_path[temp_path.size() - 1] == '/')
	{
		temp_path = temp_path.substr(0, temp_path.size() - 1);
	}
	loc.setPath(temp_path);
	++i;

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
			if (i >= tokens.size() || tokens[i] == ";") {
				CODE_ERR("Missing value for 'allow_methods' directive in location block");
			}
			std::vector<std::string> methods;
			while (i < tokens.size() && tokens[i] !=  ";" && !isLocationDirective(tokens[i]))
				methods.push_back(tokens[i++]);

			if (methods.empty())
				CODE_ERR("allow_methods directive requires at least one method.");

			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'allow_methods' directive");
			}

			loc.setMethods(methods);
		}
		else if (key == "root")
		{
			if (i >= tokens.size()) {
				CODE_ERR("Missing value for 'root' directive in location block");
			}
			loc.setRoot(tokens[i++]);

			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'root' directive");
			}
		}
		else if (key == "index")
		{
			if (i >= tokens.size() || tokens[i] == ";") {
				CODE_ERR("Missing value for 'index' directive in location block");
			}
			std::vector<std::string> index_vec;
			while (i < tokens.size() && tokens[i] != ";" && !isLocationDirective(tokens[i])) {
				index_vec.push_back(tokens[i++]);
			}

			if (index_vec.empty()) {
				index_vec.push_back("index.html");
				Logger::getInstance().warning("Empty 'index' directive in Location block. Setting default value.");
			}

			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'index' directive");
			}

			loc.setIndex(index_vec);
		}
		else if (key == "autoindex")
		{
			if (i >= tokens.size() || tokens[i] == ";") {
				CODE_ERR("Missing value for 'autoindex' directive");
			}
			std::string autoindex_value = tokens[i++];
			if (autoindex_value == "true" || autoindex_value == "on")
				loc.setAutoindex(AINDX_LOC_ON);
			else if (autoindex_value == "false" || autoindex_value == "off")
				loc.setAutoindex(AINDX_LOC_OFF);
			else {
				loc.setAutoindex(AINDX_DEF_OFF);
				Logger::getInstance().warning("Invalid argument for 'autoindex' directive. Setting default = false.");
			}
		
			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'autoindex' directive.");
			}
		}
		else if (key == "redirect") 
		{
			if (i >= tokens.size()) {
				CODE_ERR("Missing value for 'redirect' directive");
			}
			loc.setRedirect(tokens[i++]);
			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'redirect' directive.");
			}
        }
        else if (key == "cgi_extension")
		{
			if (i >= tokens.size()) {
				CODE_ERR("Missing value for 'cgi_extension' directive");
			}
			loc.setCgixtension(tokens[i++]);
			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'cgi_extension' directive.");
			}
        }
        else if (key == "allow_upload")
		{
            if (i < tokens.size()) {
                std::string value = tokens[i++];
                loc.setAllowUpload(value == "on" || value == "true");
            }
			
			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'allow_upload' directive.");
			}
        }
		else if (key == "error_page")
		{
			std::vector<int> codes;
			while (i < tokens.size() && tokens[i] != ";" && !isLocationDirective(tokens[i]) && isdigit(tokens[i][0]))
				codes.push_back(to_int(tokens[i++]));
			
			if (i < tokens.size() && tokens[i] != ";")
			{
				std::string error_page_path = tokens[i++];

				for (size_t j = 0; j < codes.size(); ++j) {
					loc.setErrorPage(codes[j], error_page_path); 
				}
			}

			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'error_page' directive.");
			}
		}
		else if (key == "client_max_body_size") {
			if (i >= tokens.size() || tokens[i] == ";") {
				CODE_ERR("Missing value for 'client_max_body_size' directive");
			}
			std::string value = tokens[i++];
			size_t limit = str_to_sizet(value, ULONG_MAX);
			loc.setMaxBodySize(limit);

			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'client_max_body_size' directive.");
			}
		}
		else if (key == "alias"){
			if (i >= tokens.size()) {
				CODE_ERR("Missing value for 'alias' directive");
			}
			loc.setAlias(tokens[i++]);
			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'alias' directive.");
			}
		}
		else
		{
			Logger::getInstance() << "Location directive not supported: " << key << std::endl;
			
			while (i < tokens.size() && tokens[i] != ";" && tokens[i] != "}") {
				++i;
			}
			
			if (i < tokens.size() && tokens[i] == ";") {
				++i;
			}
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

	if (tokens.empty())
		CODE_ERR("Config file empty.");

	ParsedServer server;

	i = expect(tokens, i, "server");
	i = expect(tokens, i, "{");
	while (i < tokens.size() && tokens[i] != "}")
	{
		if (i >= tokens.size()) break;
		std::string key = tokens[i++];

		if (i >= tokens.size() && key != "}")
		{
			CODE_ERR("Unexpected end of tokens in server block after '" + key + "'. Missing '}'");
		}

		if (key == "listen")
		{
			std::vector<std::string> listen_tokens;

			while (i < tokens.size() && tokens[i] != ";" && !isServerDirective(tokens[i]))
			{
				int temp_port;
				std::stringstream ss(tokens[i]);
				ss >> temp_port;

				// TODO validate range of port value

				listen_tokens.push_back(tokens[i++]);
			}

			if (i < tokens.size() && tokens[i] == ";")
			{
				++i;
			}
			else
			{
				CODE_ERR("Missing ';' after 'listen' directive.");
			}

			Listen ld = parse_listen(listen_tokens);
			server.listens.push_back(ld);
			// ++i; // Este ++i es extra y causa el salto de tokens
		}
		else if (key == "server_name")
		{
			if (i >= tokens.size() || tokens[i] == ";") {
				CODE_ERR("Missing value for 'server_name' directive");
			}
			std::vector<std::string> names;

			while (i < tokens.size() && tokens[i] != ";" && !isServerDirective(tokens[i]))
				names.push_back(tokens[i++]);
			
			server.server_names = names;

			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'server_name' directive.");
			}
		}
		else if (key == "root") {
			if (i >= tokens.size() || tokens[i] == ";") {
				CODE_ERR("Missing value for 'root' directive");
			}
			server.root = wss::guarantee_absolute_path(tokens[i++]);
			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'root' directive.");
			}
		}
		else if (key == "index")
		{
			if (i >= tokens.size() || tokens[i] == ";") {
				CODE_ERR("Missing value for 'index' directive in server block");
			}
			std::vector<std::string> index_files;

			while (i < tokens.size() && tokens[i] != ";" && !isServerDirective(tokens[i]))
				index_files.push_back(tokens[i++]);
			
			if (index_files.empty()) {
				index_files.push_back("index.html");
				Logger::getInstance().warning("Empty 'index' directive in Server block. Setting default value.");
			}

			server.index_files = index_files;

			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'index' directive.");
			}
		}
		else if (key == "error_page")
		{
			std::vector<int>codes;
		
			while (i < tokens.size() && tokens[i] != ";" && !isServerDirective(tokens[i]) && isdigit(tokens[i][0]))
				codes.push_back(to_int(tokens[i++]));

			if (codes.empty())
				CODE_ERR("'error_page' directive requieres at least one error code.");

			if (i >= tokens.size() || tokens[i] == ";" || isServerDirective(tokens[i]))
				CODE_ERR("'error_page' directive requires a file path after error codes");


			std::string error_page_path = tokens[i++];

			for (size_t j = 0; j < codes.size(); ++j)
				server.error_pages[codes[j]] = error_page_path;

			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'error_page' directive.");
			}
		}
		else if (key == "allow_methods")
		{
			if (i >= tokens.size() || tokens[i] == ";") {
				CODE_ERR("Missing value for 'allow_methods' directive in server block");
			}

			std::vector<std::string> methods;

			while (i < tokens.size() && tokens[i] != ";" && !isServerDirective(tokens[i]))
				methods.push_back(tokens[i++]);

			if (methods.empty())
				CODE_ERR("'allow_methods directive requires at least one method.");

			server.allow_methods = methods;

			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'allow_methods' directive.");
			}

		}
		else if (key == "autoindex")
		{
			if (i >= tokens.size() || tokens[i] == ";") {
				CODE_ERR("Missing value for 'autoindex' directive in server block");
			}
			std::string autoindex_value = tokens[i++];
			if (autoindex_value == "true" || autoindex_value == "on")
				server.autoindex = AINDX_SERV_ON;
			else if (autoindex_value == "false" || autoindex_value == "off")
				server.autoindex = AINDX_SERV_OFF;
			else {
				server.autoindex = AINDX_DEF_OFF;
				std::cout << "Invalid argument for 'autoindex' directive. Setting default = false." << std::endl;
			}

			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'autoindex' directive.");
			}
		}
		else if (key == "client_max_body_size") {
			if (i >= tokens.size() || tokens[i] == ";") {
				CODE_ERR("Missing value for 'client_max_body_size' directive in server block");
			}
			server.client_max_body_size = tokens[i++];

			if (i < tokens.size() && tokens[i] == ";") ++i;
			else {
				CODE_ERR("Missing ';' after 'client_max_body_size' directive.");
			}
		}
		else if (key == "location")
		{
			Location loc = parseLocation(tokens, i);
			std::string map_key = loc.getPath();
			if (loc.getMatchType() == Location::EXACT) {
				map_key = "=" + map_key;
			}

			if (server.locations.find(map_key) != server.locations.end()) {
				CODE_ERR("Duplicate location '" + map_key + "' in location block found, KILLING.");
			}

			server.locations[map_key] = loc;
		}
		else if (key == "server")
		{
			CODE_ERR("'server' directive cannot be nested inside another server block");
		}
		else
		{
			Logger::getInstance() << "Directive not supported: " << key << std::endl;
			while (i < tokens.size() && !tokens[i].empty() &&
				(tokens[i] != ";" && tokens[i] != "}"))
			{
				++i;
			}
			if (i < tokens.size() && tokens[i] == ";")
			{
				++i;
			}
		}
	}
	// Unir las condiciones
	if (i >= tokens.size()) {
		CODE_ERR("Unexpected end of file - server block not closed (missing '}')");
	}
	if (tokens[i] != "}") {
		CODE_ERR("Expected '}' to close server block, got: '" + tokens[i] + "'");
	}
	++i;

	applyAutoindex(server);
	applyAllowMethods(server);
	applyIndexFiles(server);

	if (server.listens.empty())
	{
		// CODE_ERR("Server block requieres at leas tn 'listen' directive");
		Logger::getInstance().warning("Sever block without 'listen' directive. Using default: '0.0.0.0:8080");
		Listen default_listen;
		default_listen.host = "0.0.0.0";
		default_listen.port = 8080;
		server.listens.push_back(default_listen);
	}

	return (server);
}

/*

Posibles mejoras de robustez de la validacion en server validator
 Códigos de error HTTP válidos (100-599)
 Tamaños de client_max_body_size válidos
*/

std::vector<ParsedServer> parseConfig(const std::vector<std::string> &tokens)
{
	std::vector<ParsedServer> servers;

	if (tokens.empty())
	{
		CODE_ERR("Config file is empty or containes only comments");
	}

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

int parseProcess(int argc, char **argv, ParsedServers& parsedConfig) {

    if (argc > 2)
	{
		Logger::getInstance().error("This program start with arguments ---> ./webserver [filename]\n");
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
            Logger::getInstance().error("Cannot open config file: " + configFile);
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
		Logger::getInstance().error(std::string("PARSING ERROR: ") + e.what());
        return (1);
    }
    
    return (0);
}