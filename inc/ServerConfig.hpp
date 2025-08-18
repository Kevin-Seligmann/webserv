#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "Location.hpp"
#include "Parsed.hpp"
#include "Enums.hpp"

struct ServerConfig {
    std::vector<std::string> server_names;
    std::string root;
    std::vector<std::string> index_files;
    std::map<int, std::string> error_pages;
    std::vector<std::string> allow_methods;
    AutoIndexState autoindex;
    size_t client_max_body_size;
    std::map<std::string, Location> locations;

    ServerConfig();
    ServerConfig(const ParsedServer& parsed);
    ServerConfig(const ServerConfig& other);
    ServerConfig& operator=(const ServerConfig& other);

    bool matchesServerName(const std::string& hostname) const;
    Location* findLocation(const std::string& path, bool resolve_index = true) const;
    std::string getErrorPage(int error_code) const;
    bool isMethodAllowed(const std::string& method) const;
    bool isAutoIndexEnabled() const;
    std::string getClientMaxBodySizeString() const;
    std::string getDebugInfo() const;

private:
    size_t parseBodySize(const std::string& size_str) const;
    Location* resolveIndexAndRematch(const std::string& path, Location* original_location) const;
};

std::ostream& operator<<(std::ostream& os, const ServerConfig& config);

#endif