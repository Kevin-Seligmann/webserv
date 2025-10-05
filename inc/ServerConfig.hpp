#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "Location.hpp"
#include "Parsed.hpp"
#include "Enums.hpp"

struct ServerConfig {
    std::vector<std::string>        server_names;
    std::string                     root;
    std::vector<std::string>        index_files;
    std::map<int, std::string>      error_pages;
    std::vector<std::string>        allow_methods;
    AutoIndexState                  autoindex;
    size_t                          client_max_body_size;
    std::map<std::string, Location> locations;
    bool                            allow_upload;

    ServerConfig();
    ServerConfig(const ParsedServer& parsed);
    ServerConfig(const ServerConfig& other);
    ServerConfig& operator=(const ServerConfig& other);

    bool matchesServerName(const std::string& hostname) const;
    Location* findLocation(const std::string& path) const;
//    Location* resolveRequest(const std::string& request_path, std::string& final_path) const;
    
    bool isMethodAllowed(const std::string& method) const;
    bool isDefaultServer() const;

    bool getAutoindex() const;
    AutoIndexState getAutoindex(const Location* location) const;
    const std::vector<std::string>& getServerNames() const { return server_names; }
    const std::string& getRoot() const { return root; }
    const std::vector<std::string>& getIndexFiles() const { return index_files; }
    std::string getErrorPage(int error_code, const Location* location = NULL) const;
    const std::vector<std::string>& getAllowMethods() const { return allow_methods; }
    const std::vector<std::string>& getAllowMethods(const Location* location) const;
    size_t getClientMaxBodySize() const { return client_max_body_size; }
    const std::map<std::string, Location>& getLocations() const;
    bool getAllowUpload() const { return allow_upload; }

private:
    std::vector<std::string> getIndexes(const Location* loc) const;
    std::string getDocRoot(const Location* loc) const;
};

#endif