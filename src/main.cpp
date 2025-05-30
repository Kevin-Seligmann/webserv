/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 12:09:35 by irozhkov          #+#    #+#             */
/*   Updated: 2025/05/30 14:11:52 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Parsed.hpp"
#include "ServerValidator.hpp"

/*struct LocationConfig
{
	std::string path;
	std::string root;
	std::string index;
	std::vector<std::string> allow_methods;
	bool autoindex;
	std::string return_path;
	bool allow_upload;
	std::string upload_dir;
	std::map<std::string, std::string> cgi;

	LocationConfig()
		: autoindex(false), allow_upload(false) {}
};

struct ListenDirective
{
	std::string ip;
	int port;
	bool is_default;

	ListenDirective()
        : ip("0.0.0.0"), port(8080), is_default(false) {}

	std::string to_string() const 
	{
		std::ostringstream oss;
		oss << ip << ":" << port;
		if (is_default) oss << " [default_server]";
		return oss.str();
	}
};

struct ServerConfig
{
	std::vector<ListenDirective> listens;
	std::vector<std::string> server_names;
	std::string root;
	std::map<int, std::string> error_pages;
	std::vector<std::string> allow_methods;
	bool autoindex;
	std::string client_max_body_size;
	std::map<std::string, LocationConfig> locations;

	ServerConfig() : autoindex(false) {}
};

std::vector<std::string> tokenize(const std::string& content)
{
	std::vector<std::string> tokens;
	std::string current;
	bool is_comment = false;

	for (size_t i = 0; i < content.length(); ++i)
	{
		char c = content[i];

		if (c == '#') {
            is_comment = true;
            continue;
        }

        if (is_comment) {
            if (c == '\n') {
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

LocationConfig parseLocation(const std::vector<std::string> &tokens, size_t &i) {
    LocationConfig loc;
    loc.path = tokens[i++];
    i = expect(tokens, i, "{");
    while (tokens[i] != "}") {
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
        else if (key == "cgi") {
            std::string ext = tokens[i++];
            std::string path = tokens[i++];
            loc.cgi[ext] = path;
        }
        if (tokens[i] == ";") ++i;
    }
    ++i;
    return (loc);
}

int to_int(const std::string& s) {
    std::stringstream ss(s);
    int result;
    ss >> result;
    return (result);
}

ListenDirective parse_listen(const std::vector<std::string>& tokens) {
    ListenDirective ld;

	for (size_t i = 0; i < tokens.size(); ++i) 
	{
    	const std::string& token = tokens[i];
        if (token == "default_server") {
            ld.is_default = true;
        } else {
            size_t colon = token.find(':');
            if (colon != std::string::npos) {
                ld.ip = token.substr(0, colon);
                ld.port = to_int(token.substr(colon + 1));
            } else if (std::isdigit(token[0])) {
                ld.port = to_int(token);
            } else {
                ld.ip = token;
            }
        }
    }

    return (ld);
}

ServerConfig parseServer(const std::vector<std::string> &tokens, size_t &i) {
    ServerConfig server;
    i = expect(tokens, i, "server");
    i = expect(tokens, i, "{");
    while (tokens[i] != "}") {
        std::string key = tokens[i++];
		if (key == "listen")
		{
			std::vector<std::string> listen_tokens;
			while (tokens[i] != ";") 
			{
				listen_tokens.push_back(tokens[i++]);
			}
			++i;
			ListenDirective ld = parse_listen(listen_tokens);
			server.listens.push_back(ld);
        } else if (key == "server_name") {
            while (tokens[i] != ";") server.server_names.push_back(tokens[i++]);
        } else if (key == "root") server.root = tokens[i++];
        else if (key == "error_page") {
            int code = to_int(tokens[i++]);
            while (isdigit(tokens[i][0])) code = to_int(tokens[i++]);
            server.error_pages[code] = tokens[i++];
        } else if (key == "allow_methods") {
            while (tokens[i] != ";") server.allow_methods.push_back(tokens[i++]);
        } else if (key == "autoindex") server.autoindex = (tokens[i++] == "true");
        else if (key == "client_max_body_size") server.client_max_body_size = tokens[i++];
        else if (key == "location") {
            LocationConfig loc = parseLocation(tokens, i);
            server.locations[loc.path] = loc;
        }
        if (tokens[i] == ";") ++i;
    }
    ++i;
    return server;
}

std::vector<ServerConfig> parseConfig(const std::vector<std::string> &tokens) {
    std::vector<ServerConfig> servers;
    size_t i = 0;
    while (i < tokens.size()) {
        if (tokens[i] == "server") {
            servers.push_back(parseServer(tokens, i));
        } else {
            throw std::runtime_error("Unknown directive outside server block: " + tokens[i]);
        }
    }
    return servers;
}

void printLocationConfig(const LocationConfig& loc) {
    std::cout << "    Path: " << loc.path << std::endl;
    std::cout << "    Root: " << loc.root << std::endl;
    std::cout << "    Index: " << loc.index << std::endl;
    std::cout << "    Autoindex: " << (loc.autoindex ? "true" : "false") << std::endl;
    std::cout << "    Return Path: " << loc.return_path << std::endl;
    std::cout << "    Allow Upload: " << (loc.allow_upload ? "true" : "false") << std::endl;
    std::cout << "    Upload Dir: " << loc.upload_dir << std::endl;

    std::cout << "    Allow Methods:" << std::endl;
    for (size_t i = 0; i < loc.allow_methods.size(); ++i)
        std::cout << "      - " << loc.allow_methods[i] << std::endl;

    std::cout << "    CGI Handlers:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it)
        std::cout << "      " << it->first << " => " << it->second << std::endl;
}

void printServerConfig(const ServerConfig& config) {
    std::cout << "\n=== ServerConfig ===" << std::endl;

    std::cout << "Listen Directives:" << std::endl;
    for (size_t i = 0; i < config.listens.size(); ++i)
        std::cout << "  - " << config.listens[i].to_string() << std::endl;

    std::cout << "Server Names:" << std::endl;
    for (size_t i = 0; i < config.server_names.size(); ++i)
        std::cout << "  - " << config.server_names[i] << std::endl;

    std::cout << "Root: " << config.root << std::endl;

    std::cout << "Error Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = config.error_pages.begin();
         it != config.error_pages.end(); ++it)
        std::cout << "  " << it->first << " => " << it->second << std::endl;

    std::cout << "Allowed Methods:" << std::endl;
    for (size_t i = 0; i < config.allow_methods.size(); ++i)
		std::cout << "  - " << config.allow_methods[i] << std::endl;
    
	std::cout << "Autoindex: " << (config.autoindex ? "true" : "false") << std::endl;

    std::cout << "Client Max Body Size: " << config.client_max_body_size << std::endl;

    std::cout << "Locations:" << std::endl;
    for (std::map<std::string, LocationConfig>::const_iterator it = config.locations.begin();
         it != config.locations.end(); ++it) {
        std::cout << "  Location block for: " << it->first << std::endl;
        printLocationConfig(it->second);
    }

    std::cout << std::endl;
}
*/
int main(int argc, char* argv[])
{
	(void)argc;
	try
	{
		std::ifstream file(argv[1]);
		std::stringstream buffer;
		buffer << file.rdbuf();

		std::string content = buffer.str();
		std::vector<std::string> tokens = tokenize(content);

		std::vector<std::string>::iterator it;
		for (it = tokens.begin(); it != tokens.end(); ++it)
		{
			std::cout << *it << std::endl;
		}

		std::vector<ParsedServer> servers = parseConfig(tokens);

		for (size_t i = 0; i < servers.size(); ++i)
		{
			std::cout << "\n--- Server " << i + 1 << " ---\n";
			printServerConfig(servers[i]);
		}

		ServerValidator::validate(servers);
	}
	catch (const  std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
	}

	return (0);
}
