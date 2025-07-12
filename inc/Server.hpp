#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>          // Para getaddrinfo()
#include <cstring>          // Para memset()
#include <unistd.h>         // Para close()
#include <iostream>         // Para std::cout
#include "Parsed.hpp"
#include "Location.hpp"
#include "HTTPRequest.hpp"

class Server {
public:
    struct NetworkLayer {
        int socket_fd;
        struct sockaddr_in address;
        std::string host;
        int port;
        bool active;
        
        NetworkLayer();
        void setupSocket();
        void bindAndListen();
    };
    
    struct ConfigLayer {
        std::vector<std::string> server_names;
        std::vector<Location> locations;
        std::map<int, std::string> error_pages;
        std::string root;
        std::vector<std::string> index_files;
        std::vector<std::string> allow_methods;
        bool autoindex;
        size_t client_max_body_size;
        
        ConfigLayer();
        bool matchesServerName(const std::string& name) const;
        Location* findLocation(const std::string& path);
        std::string getErrorPage(int code) const;
    };

private:
    NetworkLayer _network;
    ConfigLayer _config;

public:
    Server();
    Server(const ParsedServer& parsed);
    ~Server();
    
    void start();
    int getSocketFD() const;
    bool isActive() const;
    bool matchesRequest(const HTTPRequest& request) const;
    
    // Getters para acceso transparente a los miembros
    const NetworkLayer& getNetwork() const { return _network; }
    const ConfigLayer& getConfig() const { return _config; }
    ConfigLayer& getConfig() { return _config; }
    
    // Métodos para mostrar información detallada
    void printNetworkLayer() const;
    void printConfigLayer() const;
};

#endif