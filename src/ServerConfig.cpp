// ===== ServerConfig.cpp =====

#include "ServerConfig.hpp"
#include "StringUtil.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>

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
    , allow_upload(false) // falta el allow_upload en ParsedServer
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
        default_location.setMatchType(Location::PREFIX); // QUESTION como va esto del setMatchType, contexto otrs tipos, donde se consume
        locations["/"] = default_location;
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
// Función auxiliar para debug (pon static antes de esta función)
static void debugLocationSearch(const std::string& path, const std::map<std::string, Location>& locations, 
                        const Location& location, const std::string& key) {
    Logger::getInstance().info("DEBUG: iterating key='" + key + "'");
    (void) locations;
    
    // DEBUG CRÍTICO: valores raw del enum
    Logger::getInstance().info("DEBUG: location._match_type raw value = " + 
        wss::i_to_dec(static_cast<int>(location.getMatchType())));
    Logger::getInstance().info("DEBUG: Location::PREFIX enum value = " + 
        wss::i_to_dec(static_cast<int>(Location::PREFIX)));
    Logger::getInstance().info("DEBUG: Location::EXACT enum value = " + 
        wss::i_to_dec(static_cast<int>(Location::EXACT)));
    
    bool match_type_ok = (location.getMatchType() == Location::PREFIX);
    bool matches_path_ok = location.matchesPath(path);
    
    Logger::getInstance().info("DEBUG: match_type_ok=" + 
        std::string(match_type_ok ? "TRUE" : "FALSE"));
    Logger::getInstance().info("DEBUG: matches_path_ok=" + 
        std::string(matches_path_ok ? "TRUE" : "FALSE"));
    
    if (match_type_ok && matches_path_ok) {
        Logger::getInstance().info("DEBUG: ENTERING IF BLOCK!");
    } else {
        Logger::getInstance().info("DEBUG: NOT ENTERING IF - one condition failed");
    }
}

Location* ServerConfig::findLocation(const std::string& path, bool resolve_index) const {
    // Debug inicial
    Logger::getInstance() << ("DEBUG findLocation: searching for '" + path + "'");
    Logger::getInstance() << ("DEBUG: map size = " + wss::i_to_dec(locations.size()));
    
    // Buscar match exacto primero
    std::string exact_key = "=" + path;
    std::map<std::string, Location>::const_iterator exact_it = locations.find(exact_key);
    if (exact_it != locations.end() && exact_it->second.getMatchType() == Location::EXACT) {
        Logger::getInstance() << ("DEBUG: found EXACT match");
        Location* exact_location = const_cast<Location*>(&exact_it->second);
        
        if (resolve_index && !path.empty() && path[path.length() - 1] == '/') {
            return resolveIndexAndRematch(path, exact_location);
        }
        return exact_location;
    }
    
    // Debug para map entry encontrado
    if (exact_it != locations.end()) {
        Logger::getInstance() << ("DEBUG: found map entry, match_type = " + 
            std::string(exact_it->second.getMatchType() == Location::EXACT ? "EXACT" : "PREFIX"));
    }
    
    // Buscar PREFIX matches
    Location* best_match = NULL;
    size_t best_length = 0;
    
    for (std::map<std::string, Location>::const_iterator it = locations.begin();
         it != locations.end(); ++it) {
        
        const Location& location = it->second;
        
        // Debug call
        debugLocationSearch(path, locations, location, it->first);
        
        // Lógica funcional
        if (location.getMatchType() == Location::PREFIX && location.matchesPath(path)) {
            size_t match_length = it->first.length();
            
            // Debug interno del match
            Logger::getInstance() << "DEBUG: match_length=" + wss::i_to_dec(match_length);
            Logger::getInstance() << "DEBUG: best_length=" + wss::i_to_dec(best_length);
            Logger::getInstance() << "DEBUG: (match_length > best_length)=" + 
                std::string(match_length > best_length ? "TRUE" : "FALSE");
            
            if (match_length > best_length) {
                Logger::getInstance() << "DEBUG: ASSIGNING best_match!";
                best_match = const_cast<Location*>(&location);
                best_length = match_length;
            }
        }
    }
    
    if (best_match && resolve_index && !path.empty() && path[path.length() - 1] == '/') {
        return resolveIndexAndRematch(path, best_match);
    }
    
    // Debug final
    Logger::getInstance() << "DEBUG: final best_match=" + 
        std::string(best_match ? "NOT_NULL" : "NULL");
    
    return best_match;
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

// getAllowMethods
const std::vector<std::string>& ServerConfig::getAllowMethods(const Location* location) const {
    if (location && !location->getMethods().empty()) {
        return location->getMethods();
    }
    
    return allow_methods;
}


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

// getDebugInfo
std::string ServerConfig::getDebugInfo() const {
    std::stringstream ss;
    
    ss << "ServerConfig {\n";
    ss << "  server_names: [";
    for (size_t i = 0; i < server_names.size(); ++i) {
        ss << "\"" << server_names[i] << "\"";
        if (i < server_names.size() - 1) ss << ", ";
    }
    ss << "]\n";
    
    ss << "  root: \"" << root << "\"\n";
    ss << "  autoindex: " << (getAutoindex() ? "enabled" : "disabled") << "\n";
    ss << "  client_max_body_size: " << getClientMaxBodySizeString() << "\n";
    
    ss << "  index_files: [";
    for (size_t i = 0; i < index_files.size(); ++i) {
        ss << "\"" << index_files[i] << "\"";
        if (i < index_files.size() - 1) ss << ", ";
    }
    ss << "]\n";
    
    ss << "  allow_methods: [";
    for (size_t i = 0; i < allow_methods.size(); ++i) {
        ss << "\"" << allow_methods[i] << "\"";
        if (i < allow_methods.size() - 1) ss << ", ";
    }
    ss << "]\n";
    
    ss << "  locations: " << locations.size() << " locations\n";
    ss << "  error_pages: " << error_pages.size() << " custom pages\n";
    ss << "}";
    
    return ss.str();
}

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

Location* ServerConfig::resolveIndexAndRematch(const std::string& path, Location* original_location) const {
    for (std::vector<std::string>::const_iterator it = index_files.begin();
         it != index_files.end(); ++it) {
        
        std::string index_path = path + *it;
        
        Location* new_location = findLocation(index_path, false);
        if (new_location && new_location != original_location) {
            return new_location;
        }
    }
    
    return original_location;
}

std::ostream& operator<<(std::ostream& os, const ServerConfig& config) {
    os << config.getDebugInfo();
    return os;
}

// para debug
std::string ServerConfig::getServerNamesString() const {
    if (server_names.empty()) {
        return "_";
    }
    std::string result;
    for (size_t i = 0; i < server_names.size(); ++i) {
        result += server_names[i];
        if (i < server_names.size() - 1) {
            result += ", ";
        }
    }
    return result;
}

bool ServerConfig::isDefaultServer() const {
    return server_names.empty() || 
            (server_names.size() == 1 && server_names[0] == "_");
}