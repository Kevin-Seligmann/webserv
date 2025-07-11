#include "Server.hpp"
#include <stdexcept>
#include <unistd.h>
#include <cstring>
#include <cstdlib>  // Para std::atoi
#include <iostream>
#include <arpa/inet.h>  // Para ntohs
#include <iostream>  // Para std::cout

// NetworkLayer Implementation
Server::NetworkLayer::NetworkLayer() 
    : socket_fd(-1), host("0.0.0.0"), port(80), active(false) {
    std::memset(&address, 0, sizeof(address));
}

void Server::NetworkLayer::setupSocket() {

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(socket_fd);
        throw std::runtime_error("Failed to set socket options");
    }
    
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    
    if (host == "0.0.0.0" || host.empty()) { // todas las ips
        address.sin_addr.s_addr = INADDR_ANY;
        std::cout << "Binding to all interfaces (0.0.0.0):" << port << std::endl;
    }
    else { // solo una ip
        struct addrinfo hints, *result; 
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        int ret = getaddrinfo(host.c_str(), NULL, &hints, &result);
        if (ret != 0) {
            close(socket_fd);
            throw std::runtime_error("Invalid host address: " + host);
        }
        
        struct sockaddr_in* addr_in = (struct sockaddr_in*)result->ai_addr;
        address.sin_addr = addr_in->sin_addr;
        
        freeaddrinfo(result);
        std::cout << "Binding to specific host " << host << ":" << port << std::endl;
    }
}

void Server::NetworkLayer::bindAndListen() {
    if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(socket_fd);
        throw std::runtime_error("Failed to bind socket");
    }
    
    if (listen(socket_fd, 128) < 0) {
        close(socket_fd);
        throw std::runtime_error("Failed to listen on socket");
    }
    
    active = true;
}

// ConfigLayer Implementation
Server::ConfigLayer::ConfigLayer() 
    : autoindex(false), client_max_body_size(1024 * 1024) {
}

bool Server::ConfigLayer::matchesServerName(const std::string& name) const {
    if (server_names.empty()) return true;  // Default server
    
    for (std::vector<std::string>::const_iterator it = server_names.begin();
         it != server_names.end(); ++it) {
        if (*it == name) return true;
    }
    return false;
}

Location* Server::ConfigLayer::findLocation(const std::string& path) {
    Location* best_match = NULL;
    size_t best_length = 0;
    
    for (std::vector<Location>::iterator it = locations.begin();
         it != locations.end(); ++it) {

        if (it->matchesPath(path)) {
            size_t match_length = it->getPath().length();
            if (match_length > best_length) {
                best_match = &(*it);
                best_length = match_length;
            }
        }
    }
    return best_match;
}

std::string Server::ConfigLayer::getErrorPage(int code) const {
    std::map<int, std::string>::const_iterator it = error_pages.find(code);
    if (it != error_pages.end()) {
        return it->second;
    }
    return "";  // Default error page
}

// Server Implementation
Server::Server() {
}

Server::Server(const ParsedServer& parsed) {
    if (!parsed.listens.empty()) {
        _network.host = parsed.listens[0].host;
        _network.port = parsed.listens[0].port;
    }
    
    _config.server_names = parsed.server_names;
    _config.error_pages = parsed.error_pages;
    _config.root = parsed.root;
    _config.index_files = parsed.index_files;
    _config.allow_methods = parsed.allow_methods;
    _config.autoindex = parsed.autoindex;
    
    // Convert client_max_body_size from string
    if (!parsed.client_max_body_size.empty()) {
        // Simple conversion - you might want to handle units (K, M, G)
        _config.client_max_body_size = std::atoi(parsed.client_max_body_size.c_str());
    }
    
    // Convert locations map to vector
    for (std::map<std::string, Location>::const_iterator it = parsed.locations.begin();
         it != parsed.locations.end(); ++it) {
        _config.locations.push_back(it->second);
    }
}

Server::~Server() {
    if (_network.socket_fd >= 0) {
        close(_network.socket_fd);
    }
}

void Server::start() {
    _network.setupSocket();
    _network.bindAndListen();
}

int Server::getSocketFD() const {
    return _network.socket_fd;
}

bool Server::isActive() const {
    return _network.active;
}

bool Server::matchesRequest(const HTTPRequest& request) const {
    // Check if port matches
    if (request.get_port() != _network.port) {
        return false;
    }
    
    // Check server name
    std::string host = request.get_host();
    return _config.matchesServerName(host);
}

void Server::printNetworkLayer() const {
    std::cout << "📡 NETWORK LAYER (Real Server Members):" << std::endl;
    std::cout << "  socket_fd: " << _network.socket_fd << std::endl;
    std::cout << "  host: " << _network.host << std::endl;
    std::cout << "  port: " << _network.port << std::endl;
    std::cout << "  active: " << (_network.active ? "true" : "false") << std::endl;
    std::cout << "  address family: " << _network.address.sin_family << std::endl;
    std::cout << "  address port: " << ntohs(_network.address.sin_port) << std::endl;
}

void Server::printConfigLayer() const {
    std::cout << "⚙️  CONFIG LAYER (Real Server Members):" << std::endl;
    
    std::cout << "  server_names: [";
    for (size_t i = 0; i < _config.server_names.size(); ++i) {
        std::cout << "\"" << _config.server_names[i] << "\"";
        if (i < _config.server_names.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    std::cout << "  root: \"" << _config.root << "\"" << std::endl;
    std::cout << "  autoindex: " << (_config.autoindex ? "true" : "false") << std::endl;
    std::cout << "  client_max_body_size: " << _config.client_max_body_size << std::endl;
    
    std::cout << "  index_files: [";
    for (size_t i = 0; i < _config.index_files.size(); ++i) {
        std::cout << "\"" << _config.index_files[i] << "\"";
        if (i < _config.index_files.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    std::cout << "  allow_methods: [";
    for (size_t i = 0; i < _config.allow_methods.size(); ++i) {
        std::cout << "\"" << _config.allow_methods[i] << "\"";
        if (i < _config.allow_methods.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    std::cout << "  locations.size(): " << _config.locations.size() << std::endl;
    std::cout << "  error_pages.size(): " << _config.error_pages.size() << std::endl;
}
