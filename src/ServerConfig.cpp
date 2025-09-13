// ===== ServerConfig.cpp =====

#include "ServerConfig.hpp"
#include "StringUtil.hpp"
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
	, allow_upload(false) // TODO: falta el allow_upload en ParsedServer
{
	if (index_files.empty()) {
		index_files.push_back("index.html");
		index_files.push_back("index.htm");
	}
	
	if (allow_methods.empty()) {
		allow_methods.push_back("GET");
		allow_methods.push_back("POST");
		allow_methods.push_back("DELETE");
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

	// Buscar match exacto primero
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

// RESOLVE REQUEST
Location* ServerConfig::resolveRequest(const std::string& request_path, std::string& final_path) const {

	static int resolve_count = 0;
	Logger::getInstance() << "RESOLVE_REQUEST #" << ++resolve_count << " path = '" << request_path << "'" << std::endl; 

	if (resolve_count > 10)
	{
		Logger::getInstance().error("RESOLVE REQUEST called too many times - KILLING");
		exit(1);
	}

	final_path = request_path;
	Location* location = findLocation(request_path);

	Logger::getInstance() << "Matching inicial: " << request_path << " -> "
						  << (location ? location->getPath() : "NULL") << std::endl;
	
	// Verificar si es un directorio física
	if (!request_path.empty() && request_path[request_path.length() - 1] != '/') {
		std::string try_dir_path = request_path + "/";
		Location* dir_location = findLocation(try_dir_path);

		if (dir_location && dir_location != location) {
			Logger::getInstance() << "Found directory Location: " << try_dir_path << std::endl;

			std::string phys_path = getDocRoot(dir_location) + request_path;
			struct stat statbuf;
			if (stat(phys_path.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
				final_path = try_dir_path;
				return dir_location;
			}

			Logger::getInstance() << "No necesita resolucion de indices (termina != '/')" << std::endl;
			return location;
		}
	}

	std::vector<std::string> indexes = getIndexes(location);
	std::string doc_root = getDocRoot(location);

	Logger::getInstance() << "Intentando con " << indexes.size() << " index files" << std::endl;

	for (size_t i = 0; i < indexes.size(); ++i) {
		if (indexes[i].empty()) continue;

		std::string try_logic_path = request_path + indexes[i];
		std::string try_physical_path = normalizePath(doc_root, try_logic_path);

		if (access(try_physical_path.c_str(), F_OK) == 0) {
			final_path = try_logic_path;
			Logger::getInstance() << "Path resuelto a " << final_path << std::endl;

			Location* new_location = findLocation(final_path);
			if (new_location != location) {
				Logger::getInstance() << "Rematching (location diferente para final_path): " 
									  << final_path << " -> "
									  << (new_location ? new_location->getPath() : "NULL")
									  << std::endl;
				return new_location;
			}
			else {
				Logger::getInstance() << "Rematching con mismo location" << std::endl;
				return location;
			}
		}
	}
	Logger::getInstance() << "No se encontró archivo índice." << std::endl;
	return location;
}


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

	return ""; // TODO implementar paginas default, reemplazar aqui
}

// getAllowMethods
const std::vector<std::string>& ServerConfig::getAllowMethods(const Location* location) const {
	if (location && !location->getMethods().empty()) {
		return location->getMethods();
	}
	
	return allow_methods;
}

// getClientMaxBodySizeString
std::string ServerConfig::getClientMaxBodySizeString() const {
	if (client_max_body_size >= 1024 * 1024 * 1024) {
		return wss::i_to_dec(client_max_body_size / (1024 * 1024 * 1024)) + "G";
	} else if (client_max_body_size >= 1024 * 1024) {
		return wss::i_to_dec(client_max_body_size / (1024 * 1024)) + "M";
	} else if (client_max_body_size >= 1024) {
		return wss::i_to_dec(client_max_body_size / 1024) + "K";
	} else {
		return wss::i_to_dec(client_max_body_size);
	}
}


// PUBLICS
size_t ServerConfig::parseBodySize(const std::string& size_str) const {
	static const size_t DEFAULT_SIZE = 1048576;  // 1MB default
	static const size_t MAX_SIZE = ~(static_cast<size_t>(0));  // MAX size_t

	if (size_str.empty()) {
		return DEFAULT_SIZE;
	}

	std::string number_part;
	std::string unit_part;
	size_t i;

	// EXTRACT NUMBER
	for (i = 0; i < size_str.length() && (std::isdigit(size_str[i]) || size_str[i] == '.'); ++i) {
		number_part += size_str[i];
	}

	// EXTRACT UNIT
	while (i < size_str.length()) {
		unit_part += std::toupper(size_str[i]);
		++i;
	}

	if (number_part.empty()) {
		return DEFAULT_SIZE;
	}

	size_t base_size = static_cast<size_t>(std::atoll(number_part.c_str()));

	// PROTECT FROM VALUE = 0
	if (base_size == 0) {
		return DEFAULT_SIZE;
	}

	// UNIT CONVERTION
	if (unit_part.empty() || unit_part == "B" || unit_part == "BYTES") {
		return base_size;
	} else if (unit_part == "K" || unit_part == "KB" || unit_part == "KIB") {
		if (base_size > MAX_SIZE / 1024) return MAX_SIZE;  // PROTECT OVERFLOW
		return base_size * 1024;
	} else if (unit_part == "M" || unit_part == "MB" || unit_part == "MIB") {
		if (base_size > MAX_SIZE / (1024 * 1024)) return MAX_SIZE;  // PROTECT OVERFLOW
		return base_size * 1024 * 1024;
	} else if (unit_part == "G" || unit_part == "GB" || unit_part == "GIB") {
		if (base_size > MAX_SIZE / (1024 * 1024 * 1024)) return MAX_SIZE;  // PROTECT OVERFLOW
		return base_size * 1024 * 1024 * 1024;
	}
	
	return DEFAULT_SIZE;  // UNKNOWN UNIT
}

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

std::ostream& operator<<(std::ostream& os, const ServerConfig& config) {
	os << "DEGUB INFO SUPRESSED (implementar aquí)";
	return os;
}