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
    bool allow_upload;

    ServerConfig();
    ServerConfig(const ParsedServer& parsed);
    ServerConfig(const ServerConfig& other);
    ServerConfig& operator=(const ServerConfig& other);

    bool matchesServerName(const std::string& hostname) const;
    Location* findLocation(const std::string& path, bool resolve_index = true) const;
    bool isMethodAllowed(const std::string& method) const;
    bool getAutoindex() const;
    AutoIndexState getAutoindex(const Location* location) const;
    std::string getClientMaxBodySizeString() const;
    std::string getDebugInfo() const;
    
    
    // 1. server_names
    const std::vector<std::string>& getServerNames() const { return server_names; }
    
    // 2. root
    const std::string& getRoot() const { return root; }
    
    // 3. index_files
    const std::vector<std::string>& getIndexFiles() const { return index_files; }
    
    // 4. error_pages
    std::string getErrorPage(int error_code) const;
//  const std::string getErrorPages(int error_code, const Location* location) const;
    
    // 5. allow_methods
    const std::vector<std::string>& getAllowMethods() const { return allow_methods; }
    const std::vector<std::string>& getAllowMethods(const Location* location) const;
    
    // 6. autoindex
    // nota: ya existe getAutoindex() y getAutoindex(const Location*) arriba
    
    // 7. client_max_body_size
    size_t getClientMaxBodySize() const { return client_max_body_size; }
    
    // 8. locations
    const std::map<std::string, Location>& getLocations() const;
    
    // 9. allow_upload
    bool getAllowUpload() const { return allow_upload; }


    // Método para obtener información de debug más detallada
    std::string getServerNamesString() const {
        if (server_names.empty()) {
            return "_";  // Default server
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

    // Verificar si es el servidor por defecto
    bool isDefaultServer() const {
        return server_names.empty() || 
               (server_names.size() == 1 && server_names[0] == "_");
    }

private:
    size_t parseBodySize(const std::string& size_str) const;
    Location* resolveIndexAndRematch(const std::string& path, Location* original_location) const;
};

std::ostream& operator<<(std::ostream& os, const ServerConfig& config);

#endif