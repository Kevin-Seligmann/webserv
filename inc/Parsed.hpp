/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsed.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:51:01 by irozhkov          #+#    #+#             */
/*   Updated: 2025/05/30 19:06:39 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSED_HPP
#define PARSED_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Forward declarations
class ServersManager;

struct ParsedListen
{
	std::string ip;
	int port;
	bool is_default;

	ParsedListen()
		: ip("0.0.0.0"), port(8080), is_default(false) {}

	std::string to_string() const
	{
		std::ostringstream oss;
		oss << ip << ":" << port;
		if (is_default) oss << " [default_server]";
		return oss.str();
	}
};

struct ParsedLocations
{
	std::string								path;
	std::string								root;
	std::string								index;
	std::vector<std::string>				allow_methods;
	bool									autoindex;
	std::string                             return_path;
	bool                                    allow_upload;
	std::string                             upload_dir;
	std::map<std::string, std::string>      cgi;

	ParsedLocations()
		: autoindex(false), allow_upload(false) {}
		
	// Methods that were in the Locations class
	void setPath(const std::string &path) { this->path = path; }
	const std::string& getPath(void) const { return path; }

	void setValidMethods(const std::vector<std::string> &validMethods) { allow_methods = validMethods; }
	const std::vector<std::string>& getValidMethods(void) const { return allow_methods; }

	void setRoot(const std::string &root) { this->root = root; }
	const std::string& getRoot(void) const { return root; }

	void setAutoindex(bool autoindex) { this->autoindex = autoindex; }
	bool getAutoindex(void) const { return autoindex; }

	// CGI helper methods
	std::string getCgiExtension(void) const {
		if (!cgi.empty()) {
			std::map<std::string, std::string>::const_iterator it = cgi.begin();
			return it->first;
		}
		return "";
	}
	
	std::string getCgiPath(void) const {
		if (!cgi.empty()) {
			std::map<std::string, std::string>::const_iterator it = cgi.begin();
			return it->second;
		}
		return "";
	}

	void setRedir(const std::string &redir) { return_path = redir; }
	const std::string& getRedir(void) const { return return_path; }

	void setUploadDir(const std::string &uploadDir) { upload_dir = uploadDir; }
	const std::string& getUploadDir(void) const { return upload_dir; }
};

struct ParsedServer {
	std::vector<ParsedListen> listens;
	std::string host;
	int port;
	std::string server_name;
	std::vector<std::string> server_names;
	std::string root;
	std::vector<std::string> index_files;
	std::map<int, std::string> error_pages;
	std::vector<std::string> allow_methods;
	bool autoindex;
	std::string client_max_body_size;
	std::map<std::string, ParsedLocations> locations;

	ParsedServer()
		: port(8080), autoindex(false) {}

	ParsedServer(const std::string& host,
				 int port,
				 const std::string& server_name,
				 const std::string& root,
				 const std::vector<std::string>& index_files,
				 const std::map<int, std::string>& error_pages,
				 const std::map<std::string, ParsedLocations>& locations);

	~ParsedServer();
};

typedef std::vector<ParsedServer> ParsedServers;

// ====== PARSING ========
std::vector<std::string> tokenize(const std::string& content);

size_t expect(const std::vector<std::string>& tokens, size_t i, const std::string& expected);

int to_int(const std::string& s);

ParsedListen parse_listen(const std::vector<std::string>& tokens);

ParsedServer parseServer(const std::vector<std::string>& tokens, size_t& i);

std::vector<ParsedServer> parseConfig(const std::vector<std::string>& tokens);

// ====== PROCESS ========
int parseProcess(int argc, char **argv, ParsedServers& parsedConfig);

void showServers(ServersManager ws);

#endif