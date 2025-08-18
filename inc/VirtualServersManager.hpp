#ifndef VIRTUAL_SERVERS_MANAGER_HPP
#define VIRTUAL_SERVERS_MANAGER_HPP

#include "ServerConfig.hpp"
#include "HTTPRequest.hpp"
#include "RequestManager.hpp"
#include "HTTPError.hpp"
#include "ElementParser.hpp"
#include "SysBufferFactory.hpp"
#include "ResponseManager.hpp"
#include "Location.hpp"
#include "HTTPMethod.hpp"
#include "Status.hpp"
#include "Listen.hpp"
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/epoll.h>

class VirtualServersManager {
public:
    // Estructura para manejar el estado de los clientes
    struct ClientState {
        HTTPRequest     request;
        HTTPError       error;
        ElementParser   element_parser;
        RequestManager  request_manager;
        ResponseManager response_manager;
    
        enum Status {
            READING_REQUEST,
            PROCESSING_REQUEST,
            WRITING_RESPONSE,
            WAITING_FILE,
            WAITING_CGI,
            CLOSING,
            CLOSED
        };
        Status status;

        // Constructor C++98 compatible
        ClientState(int client_fd);

        bool isRequestComplete() const;
        bool hasError() const;
    
        static std::map<int, ClientState*> client_states;
        static ClientState* getOrCreateClientState(int client_fd);
        static void cleanupClientState(int client_fd);
    };

    // Estructura para representar un socket de escucha único
    struct ListenSocket {
        int socket_fd;
        Listen listen_config;
        std::vector<ServerConfig*> virtual_hosts;  // Múltiples configs por socket
        
        ListenSocket();
        ListenSocket(const Listen& config);
    };

private:
    std::vector<ServerConfig>                       _server_configs;      // Todas las configuraciones
    std::map<Listen, ListenSocket>                  _listen_sockets;      // Socket único por host:port
    std::map<Listen, std::vector<ServerConfig*> >   _virtual_hosts;       // Virtual hosts por listen
    int                                             _epoll_fd;
    std::vector<struct epoll_event>                 _events;
    std::vector<int>                                _client_fds;
    std::map<int, ListenSocket*>                    _client_to_listen_socket;

    // Métodos privados de socket management
    void setupEpoll();
    void createListenSockets();
    void setupListenSocket(ListenSocket& listen_socket);
    void bindListenSocket(ListenSocket& listen_socket);
    
    // Métodos de búsqueda
    bool isListenSocket(int fd) const;
    ListenSocket* findListenSocket(int fd);
    
    // Event handling
    void handleEvent(const struct epoll_event& event);
    void handleNewConnection(ListenSocket* listen_socket);
    void handleClientData(int client_fd);
    void disconnectClient(int client_fd);
    
    // Request processing
    void processCompleteRequest(int client_fd, HTTPRequest& request);
    ServerConfig* findServerConfigForRequest(const HTTPRequest& request, ListenSocket* listen_socket);
    Location* findLocationForRequest(const HTTPRequest& request, const ServerConfig* server_config);
    bool isMethodAllowed(const ServerConfig* server_config, const Location* location, HTTPMethod method);
    
    // Response handling
    void sendErrorResponse(int client_fd, int status_code, const std::string& message);
    void sendErrorResponse(int client_fd, const HTTPError& error);
    
    // Virtual hosts auxiliares
    bool matchesServerName(const ServerConfig* config, const std::string& host) const;
    ServerConfig* getDefaultServerConfig(ListenSocket* listen_socket) const;
    
    // CGI y static content
    bool isCgiRequest(const Location* location, const std::string& path);
    void processCgiRequest(int client_fd, HTTPRequest& request, const Location* location);
    void processStaticRequest(int client_fd, const ServerConfig* server_config, const Location* location);

public:
    VirtualServersManager();
    VirtualServersManager(const ParsedServers& configs);
    ~VirtualServersManager();
    
    void run();
    
    // Debug/información
    void printVirtualHostsInfo() const;
    void printListenSocketsInfo() const;
};

#endif