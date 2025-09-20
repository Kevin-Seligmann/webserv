#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "Location.hpp"
#include "Parsed.hpp"
#include "Enums.hpp"

struct ServerConfig {
    // Miembros públicos
    std::vector<std::string>        server_names;
    std::string                     root;
    std::vector<std::string>        index_files;
    std::map<int, std::string>      error_pages;
    std::vector<std::string>        allow_methods;
    AutoIndexState                  autoindex;
    size_t                          client_max_body_size;
    std::map<std::string, Location> locations;
    bool                            allow_upload;

    // Constructores
    ServerConfig();
    ServerConfig(const ParsedServer& parsed);
    ServerConfig(const ServerConfig& other);
    ServerConfig& operator=(const ServerConfig& other);

    // Métodos públicos
    bool matchesServerName(const std::string& hostname) const;
    Location* findLocation(const std::string& path, bool resolve_index = true) const;
    bool isMethodAllowed(const std::string& method) const;
    bool getAutoindex() const;
    AutoIndexState getAutoindex(const Location* location) const;
    std::string getClientMaxBodySizeString() const;
    std::string getErrorPage(int error_code, const Location* location = NULL) const;
    
    // Getters simples (inline)
    const std::vector<std::string>& getServerNames() const { return server_names; }
    const std::string& getRoot() const { return root; }
    const std::vector<std::string>& getIndexFiles() const { return index_files; }
    const std::vector<std::string>& getAllowMethods() const { return allow_methods; }
    const std::vector<std::string>& getAllowMethods(const Location* location) const;
    size_t getClientMaxBodySize() const { return client_max_body_size; }
    bool getAllowUpload() const { return allow_upload; }
    std::string getDebugInfo() const;

private:
    // Métodos privados (helpers)
    size_t parseBodySize(const std::string& size_str) const;
    Location* resolveIndexAndRematch(const std::string& path, Location* original_location) const;
};

std::ostream& operator<<(std::ostream& os, const ServerConfig& config);

#endif