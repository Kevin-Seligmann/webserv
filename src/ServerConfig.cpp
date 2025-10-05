// ===== ServerConfig.cpp =====

#include "ServerConfig.hpp"
#include "StringUtil.hpp"
#include "DebugView.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

ServerConfig::ServerConfig() 
	: root("/var/www/html")
	, autoindex(AINDX_DEF_OFF)
	, client_max_body_size(1048576)
	, allow_upload(false)
{
	index_files.push_back("index.html");
	index_files.push_back("index.htm");
	allow_methods.push_back("GET");
	allow_methods.push_back("POST");
	allow_methods.push_back("DELETE");
}

ServerConfig::ServerConfig(const ParsedServer& parsed) 
	: server_names(parsed.server_names)
	, root(parsed.root)
	, index_files(parsed.index_files)
	, error_pages(parsed.error_pages)
	, allow_methods(parsed.allow_methods)
	, autoindex(parsed.autoindex)
	, client_max_body_size(parseBodySize(parsed.client_max_body_size))
	, locations(parsed.locations)
	, allow_upload(false)
{
	if (index_files.empty()) {
		index_files.push_back("index.html");
		index_files.push_back("index.htm");
	}
	

	if (allow_methods.empty()) {
		allow_methods.push_back("GET");
	}
	
	if (root.empty()) {
		root = "/var/www/html";
	}

	if (locations.empty()) {
		Location default_location;
		default_location.setPath("/");
		default_location.setMatchType(Location::PREFIX);
		default_location.setMethods(allow_methods);
		locations["/"] = default_location;
	}

	// Asegurarse que ninguna location se vaya sin heredar el default
	std::map<std::string, Location>::iterator it = locations.begin();
	for (; it != locations.end(); ++it) {
		if (it->second.getMethods().empty()) {
			it->second.setMethods(allow_methods);
		}
	}
}


ServerConfig::ServerConfig(const ServerConfig& other)
	: server_names(other.server_names)
	, root(other.root)
	, index_files(other.index_files)
	, error_pages(other.error_pages)
	, allow_methods(other.allow_methods)
	, autoindex(other.autoindex)
	, client_max_body_size(other.client_max_body_size)
	, locations(other.locations)
	, allow_upload(other.allow_upload)
{
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
	if (this != &other) {
		server_names = other.server_names;
		root = other.root;
		index_files = other.index_files;
		error_pages = other.error_pages;
		allow_methods = other.allow_methods;
		autoindex = other.autoindex;
		client_max_body_size = other.client_max_body_size;
		locations = other.locations;
		allow_upload = other.allow_upload;
	}
	return *this;
}

// matchesServerName
bool ServerConfig::matchesServerName(const std::string& hostname) const {
	if (server_names.empty()) {
		return true;
	}
	
	for (std::vector<std::string>::const_iterator it = server_names.begin(); 
		 it != server_names.end(); ++it) {
		
		if (*it == "_") {
			return true;
		}
		
		if (wss::casecmp(*it, hostname)) {
			return true;
		}
	}
	
	return false;
}

// findLocation
Location* ServerConfig::findLocation(const std::string& path) const {

	// Buscar match EXACT primero
	std::string exact_key = "=" + path;
	std::map<std::string, Location>::const_iterator exact_it = locations.find(exact_key);
	if (exact_it != locations.end() && exact_it->second.getMatchType() == Location::EXACT) {
		return const_cast<Location*>(&exact_it->second);
	}
		
	// Buscar PREFIX matches
	Location* best_match = NULL;
	size_t best_length = 0;
	
	for (std::map<std::string, Location>::const_iterator it = locations.begin();
		it != locations.end(); ++it) {
		
		const Location& location = it->second;
		
		if (location.getMatchType() == Location::PREFIX && location.matchesPath(path)) {
			size_t match_length = it->first.length();
						
			if (match_length > best_length) {
				best_match = const_cast<Location*>(&location);
				best_length = match_length;
			}
		}
	}
			
	return best_match;
}

/* RESOLVE REQUEST
Location* ServerConfig::resolveRequest(const std::string& request_path, std::string& final_path) const {

	final_path = request_path;

	DEBUG_LOG(">>> in resolveRequest()");

	// try to resolve exact location first
	std::string exact_key = "=" + request_path;
	std::map<std::string, Location>::const_iterator exact_it = locations.find(exact_key);
	if (exact_it != locations.end() && exact_it->second.getMatchType() == Location::EXACT)
	{
		Location* exact_location = const_cast<Location*>(&exact_it->second);

		std::vector<std::string> indexes = getIndexes(exact_location);
		std::string doc_root = getDocRoot(exact_location);

		DEBUG_LOG(">>> resolveRequest() : indexes size : " << indexes.size());
		DEBUG_LOG(">>> resolveRequest() : indexes : " << indexes[0]);

		for (size_t i = 0; i < indexes.size(); ++i)
		{
			if (indexes[i].empty()) continue;

			std::string try_logic_path = request_path + "/" + indexes[i];
			std::string try_physical_path = normalizePath(doc_root, try_logic_path);

			DEBUG_LOG(">>> resolveRequest() : try physical path : " << try_physical_path);

			if (access(try_physical_path.c_str(), F_OK) == 0)
			{
				final_path = try_logic_path;
				return exact_location;
			}

			return exact_location;
		}
	}

	Location* location = findLocation(request_path);
	
	// Verificar si es un directorio física
	if (!request_path.empty() && request_path[request_path.length() - 1] != '/') {
		std::string try_request_path = request_path + "/";
		Location* try_location = findLocation(try_request_path);

		if (try_location && try_location != location) {

			std::string phys_path = getDocRoot(try_location) + request_path;
			struct stat statbuf;
			if (stat(phys_path.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
				final_path = try_request_path;
				return try_location;
			}
			return location;
		}
	}

	std::vector<std::string> indexes = getIndexes(location);
	std::string doc_root = getDocRoot(location);

	for (size_t i = 0; i < indexes.size(); ++i) {
		if (indexes[i].empty()) continue;

		std::string try_logic_path = request_path + indexes[i];
		std::string try_physical_path = normalizePath(doc_root, try_logic_path);

		if (access(try_physical_path.c_str(), F_OK) == 0) {
			final_path = try_logic_path;

			Location* new_location = findLocation(final_path);
			if (new_location != location) {
				return new_location;
			}
			else {
				return location;
			}
		}
	}
	return location;
}
*/

bool ServerConfig::isDefaultServer() const {
	return server_names.empty() || 
			(server_names.size() == 1 && server_names[0] == "_");
}

// isMethodAllowed
bool ServerConfig::isMethodAllowed(const std::string& method) const {
	if (allow_methods.empty()) {
		return true;
	}
	
	for (std::vector<std::string>::const_iterator it = allow_methods.begin();
		 it != allow_methods.end(); ++it) {
		if (wss::casecmp(*it, method)) {
			return true;
		}
	}
	
	return false;
}

// GETTERS

// getAutoindex methods
bool ServerConfig::getAutoindex() const {
	return autoindex == AINDX_DEF_ON;
}

AutoIndexState ServerConfig::getAutoindex(const Location* location) const {
	if (location) {
		AutoIndexState loc_state = location->getAutoindex();
		
		if (loc_state == AINDX_LOC_ON || loc_state == AINDX_LOC_OFF) {
			return loc_state;
		}
	}
	
	return autoindex;
}

// getErrorPage
std::string ServerConfig::getErrorPage(int error_code, const Location* location) const {
	
	if (location) {
		std::string loc_page = location->getErrorPage(error_code);
		if (!loc_page.empty())
			return loc_page;
	}

	std::map<int, std::string>::const_iterator it = error_pages.find(error_code);
	if (it != error_pages.end()) {
		return it->second;
	}

	return "";
}

// getAllowMethods
const std::vector<std::string>& ServerConfig::getAllowMethods(const Location* location) const {
	if (location && !location->getMethods().empty()) {
		return location->getMethods();
	}
	
	return allow_methods;
}


// PUBLICS

// getIndexes priorizado location -> server
std::vector<std::string> ServerConfig::getIndexes(const Location* loc) const {
	if (loc && !loc->getIndex().empty()) {
		return loc->getIndex();
	}
	return index_files;
}

// getDocRoot priorizado location -> server
std::string ServerConfig::getDocRoot(const Location* loc) const {
	if (loc && !loc->getRoot().empty()) {
		return loc->getRoot();
	}
	return root;
}