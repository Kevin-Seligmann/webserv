#include "VirtualServersManager.hpp"
#include "StringUtil.hpp"

// ================ STATIC MEMBER ================

std::map<int, VirtualServersManager::ClientState*> VirtualServersManager::ClientState::client_states;

// ================ CLIENT STATE IMPLEMENTATION ================

VirtualServersManager::ClientState::ClientState(int client_fd)
    : element_parser(error)
    , request_manager(request, error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
    , response_manager(request, error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
{
    status = READING_REQUEST;
}

bool VirtualServersManager::ClientState::isRequestComplete() const {
    return request_manager.request_done();
}

bool VirtualServersManager::ClientState::hasError() const {
    return error.status() != OK;
}

VirtualServersManager::ClientState* VirtualServersManager::ClientState::getOrCreateClientState(int client_fd) {
    std::map<int, ClientState*>::iterator it = client_states.find(client_fd);
    if (it != client_states.end()) {
        return it->second;
    }

    ClientState* state = new ClientState(client_fd);
    client_states[client_fd] = state;
    return state;
}

void VirtualServersManager::ClientState::cleanupClientState(int client_fd) {
    std::map<int, ClientState*>::iterator it = client_states.find(client_fd);
    if (it != client_states.end()) {
        delete it->second;
        client_states.erase(it);
    }
}

// ================ LISTEN SOCKET IMPLEMENTATION ================

VirtualServersManager::ListenSocket::ListenSocket() {
    socket_fd = -1;
    listen_config = Listen();
}

VirtualServersManager::ListenSocket::ListenSocket(const Listen& config) {
    socket_fd = -1;
    listen_config = config;
}

// ================ CONSTRUCTORS & DESTRUCTOR ================

VirtualServersManager::VirtualServersManager() {
    _epoll_fd = -1;
    _events.resize(64);
}

VirtualServersManager::VirtualServersManager(const ParsedServers& configs) {
    _epoll_fd = -1;
    _events.resize(64);

    std::cout << "Initializing VirtualServersManager with " << configs.size() 
              << " server configurations" << std::endl;

    // Procesar cada configuración parseada (C++98 compatible)
    for (std::vector<ParsedServer>::const_iterator parsed_it = configs.begin(); 
         parsed_it != configs.end(); ++parsed_it) {
        
        const ParsedServer& parsed_server = *parsed_it;
        
        std::cout << "  Processing server with " << parsed_server.listens.size() 
                  << " listen directives" << std::endl;
        
        // Crear ServerConfig a partir de ParsedServer
        ServerConfig server_config(parsed_server);
        _server_configs.push_back(server_config);
        
        // Obtener puntero al ServerConfig recién agregado (C++98 safe)
        ServerConfig* config_ptr = &_server_configs[_server_configs.size() - 1];
        
        // Procesar todas las directivas listen de este servidor
        for (std::vector<Listen>::const_iterator listen_it = parsed_server.listens.begin(); 
             listen_it != parsed_server.listens.end(); ++listen_it) {
            
            const Listen& listen_config = *listen_it;
            
            std::cout << "    Mapping to " << listen_config.host << ":" 
                      << listen_config.port << std::endl;
            
            // Agregar al mapeo de virtual hosts
            _virtual_hosts[listen_config].push_back(config_ptr);
            
            // Crear ListenSocket si no existe (C++98 compatible)
            if (_listen_sockets.find(listen_config) == _listen_sockets.end()) {
                ListenSocket new_socket(listen_config);
                _listen_sockets[listen_config] = new_socket;
                
                std::cout << "    Created new ListenSocket for " 
                          << listen_config.host << ":" << listen_config.port << std::endl;
            }
            
            // Agregar config al ListenSocket (C++98 safe access)
            std::map<Listen, ListenSocket>::iterator socket_it = _listen_sockets.find(listen_config);
            if (socket_it != _listen_sockets.end()) {
                socket_it->second.virtual_hosts.push_back(config_ptr);
            }
        }
    }
    
    std::cout << "Virtual hosts mapping completed:" << std::endl;
    std::cout << "  Total ServerConfigs: " << _server_configs.size() << std::endl;
    std::cout << "  Total Listen addresses: " << _listen_sockets.size() << std::endl;
    std::cout << "  Total Virtual host groups: " << _virtual_hosts.size() << std::endl;
}

VirtualServersManager::~VirtualServersManager() {
    std::cout << "Destroying VirtualServersManager..." << std::endl;
    
    // Limpiar mapeo de client_fd to listen_socket
    _client_to_listen_socket.clear();

    // Cerrar epoll
    if (_epoll_fd >= 0) {
        close(_epoll_fd);
        std::cout << "  Closed epoll fd: " << _epoll_fd << std::endl;
    }
    
    // Cerrar todos los sockets de escucha
    for (std::map<Listen, ListenSocket>::iterator it = _listen_sockets.begin(); 
         it != _listen_sockets.end(); ++it) {
        
        if (it->second.socket_fd >= 0) {
            close(it->second.socket_fd);
            std::cout << "  Closed socket fd: " << it->second.socket_fd 
                      << " for " << it->first.host << ":" << it->first.port << std::endl;
        }
    }
    
    // Limpiar estados de clientes
    for (std::map<int, ClientState*>::iterator client_it = ClientState::client_states.begin();
         client_it != ClientState::client_states.end(); ++client_it) {
        delete client_it->second;
    }
    ClientState::client_states.clear();
    
    std::cout << "VirtualServersManager destroyed." << std::endl;
}

// ================ SOCKET MANAGEMENT ================

void VirtualServersManager::createListenSockets() {
    std::cout << "Creating listen sockets..." << std::endl;
    
    for (std::map<Listen, ListenSocket>::iterator it = _listen_sockets.begin(); 
         it != _listen_sockets.end(); ++it) {
        
        std::cout << "Setting up socket for " << it->first.host << ":" << it->first.port << std::endl;
        
        try {
            setupListenSocket(it->second);
            bindListenSocket(it->second);
            
            std::cout << "Socket " << it->second.socket_fd << " created for " 
                      << it->first.host << ":" << it->first.port 
                      << " with " << it->second.virtual_hosts.size() << " virtual hosts" << std::endl;
                      
        } catch (const std::exception& e) {
            std::cerr << "Failed to create socket for " << it->first.host << ":" << it->first.port 
                      << " - " << e.what() << std::endl;
            throw;
        }
    }
}

void VirtualServersManager::setupListenSocket(ListenSocket& listen_socket) {
    listen_socket.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket.socket_fd < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    
    int opt = 1;
    if (setsockopt(listen_socket.socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(listen_socket.socket_fd);
        throw std::runtime_error("Failed to set socket options");
    }
}

void VirtualServersManager::bindListenSocket(ListenSocket& listen_socket) {
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    
    address.sin_family = AF_INET;
    address.sin_port = htons(listen_socket.listen_config.port);
    
    if (listen_socket.listen_config.host == "0.0.0.0" || listen_socket.listen_config.host.empty()) {
        address.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, listen_socket.listen_config.host.c_str(), &address.sin_addr) <= 0) {
            close(listen_socket.socket_fd);
            throw std::runtime_error("Invalid host address: " + listen_socket.listen_config.host);
        }
    }
    
    if (bind(listen_socket.socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(listen_socket.socket_fd);
        throw std::runtime_error("Failed to bind socket to " + listen_socket.listen_config.host + 
                                ":" + wss::i_to_dec(listen_socket.listen_config.port));
    }
    
    if (listen(listen_socket.socket_fd, 128) < 0) {
        close(listen_socket.socket_fd);
        throw std::runtime_error("Failed to listen on socket");
    }
}

void VirtualServersManager::setupEpoll() {
    std::cout << "Setting up epoll..." << std::endl;
    
    _epoll_fd = epoll_create(1);
    if (_epoll_fd < 0) {
        throw std::runtime_error("Failed to create epoll fd");
    }
    
    createListenSockets();
    
    // Agregar todos los sockets de escucha a epoll
    for (std::map<Listen, ListenSocket>::iterator it = _listen_sockets.begin(); 
         it != _listen_sockets.end(); ++it) {
        
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = it->second.socket_fd;
        
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, it->second.socket_fd, &event) < 0) {
            throw std::runtime_error("Failed to add listen socket to epoll");
        }
    }
    
    std::cout << "Epoll configured with " << _listen_sockets.size() << " listen sockets" << std::endl;
}

// ================ SEARCH METHODS ================

bool VirtualServersManager::isListenSocket(int fd) const {
    for (std::map<Listen, ListenSocket>::const_iterator it = _listen_sockets.begin(); 
         it != _listen_sockets.end(); ++it) {
        if (it->second.socket_fd == fd) {
            return true;
        }
    }
    return false;
}

VirtualServersManager::ListenSocket* VirtualServersManager::findListenSocket(int fd) {
    for (std::map<Listen, ListenSocket>::iterator it = _listen_sockets.begin(); 
         it != _listen_sockets.end(); ++it) {
        if (it->second.socket_fd == fd) {
            return &it->second;
        }
    }
    return NULL;
}

// ================ EVENT HANDLING ================

void VirtualServersManager::handleEvent(const struct epoll_event& event) {
    int fd = event.data.fd;

    if (event.events & EPOLLIN) {
        if (isListenSocket(fd)) {
            ListenSocket* listen_socket = findListenSocket(fd);
            if (listen_socket) {
                handleNewConnection(listen_socket);
            }
        } else {
            handleClientData(fd);
        }
    }
    
    if (event.events & (EPOLLHUP | EPOLLERR)) {
        if (!isListenSocket(fd)) {
            disconnectClient(fd);
        }
    }
}

void VirtualServersManager::handleNewConnection(ListenSocket* listen_socket) {
    struct sockaddr_in client_addr;
    std::memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = accept(listen_socket->socket_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
        return;
    }
    _client_to_listen_socket[client_fd] = listen_socket; 
    
    std::cout << "New client connected on FD: " << client_fd 
              << " to " << listen_socket->listen_config.host << ":" << listen_socket->listen_config.port << std::endl;
    
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = client_fd;
    
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
        close(client_fd);
        return;
    }
    
    _client_fds.push_back(client_fd);
}

void VirtualServersManager::handleClientData(int client_fd) {
    ClientState* client = ClientState::getOrCreateClientState(client_fd);

    try {
        client->request_manager.process();

        if (client->hasError()) {
            std::cerr << "Error parsing request: " << client->error.to_string() << std::endl;
            sendErrorResponse(client_fd, client->error);
            disconnectClient(client_fd);
            return;
        }

        if (client->isRequestComplete()) {
            std::cout << "Request complete for FD: " << client_fd << std::endl;

            std::map<int, ListenSocket*>::iterator map_it = _client_to_listen_socket.find(client_fd);
            if (map_it != _client_to_listen_socket.end()) {
                ListenSocket* listen_socket = map_it->second;
                ServerConfig* server_config = findServerConfigForRequest(client->request, listen_socket);
                if (server_config) {
                    Location* location = findLocationForRequest(client->request, server_config);

                    client->response_manager.set_virtual_server(server_config);
                    client->response_manager.set_location(location);
                }
                processCompleteRequest(client_fd, client->request);
                disconnectClient(client_fd);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception processing client data: " << e.what() << std::endl;
        disconnectClient(client_fd);
    }
}

void VirtualServersManager::disconnectClient(int client_fd) {
    std::cout << "Disconnecting client FD: " << client_fd << std::endl;
    
    _client_to_listen_socket.erase(client_fd);
    
    ClientState::cleanupClientState(client_fd);
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    close(client_fd);
    
    std::vector<int>::iterator it = std::find(_client_fds.begin(), _client_fds.end(), client_fd);
    if (it != _client_fds.end()) {
        _client_fds.erase(it);
    }
}

// ================ REQUEST PROCESSING ================

void VirtualServersManager::processCompleteRequest(int client_fd, HTTPRequest& request) {
    std::cout << "Processing complete request for FD: " << client_fd << std::endl;
    std::cout << "Host: " << request.get_host() << ", Path: " << request.get_path() << std::endl;
    
    // Encontrar el ListenSocket
    std::map<int, ListenSocket*>::iterator map_it = _client_to_listen_socket.find(client_fd);
    if (map_it == _client_to_listen_socket.end()) {
        std::cerr << "No listen socket found for client FD " << client_fd << std::endl;
        sendErrorResponse(client_fd, 500, "Internal Server Error");
        return;
    }

    ListenSocket* listen_socket = map_it->second;

    // Encontrar la configuración de servidor apropiada (virtual host)
    ServerConfig* server_config = findServerConfigForRequest(request, listen_socket);
    if (!server_config) {
        std::cerr << "No server config found for host " << request.get_host() << std::endl;
        sendErrorResponse(client_fd, 404, "Virtual host not found");
        return;
    }
    
    std::cout << "Selected server config for host: " << request.get_host() << std::endl;
    
    // Encontrar la location apropiada
    Location* location = findLocationForRequest(request, server_config);
    if (!location) {
        std::cerr << "No location found for path " << request.get_path() << std::endl;
        sendErrorResponse(client_fd, 404, "Location not found");
        return;
    }
    
    // Verificar si el método está permitido
    if (!isMethodAllowed(server_config, location, request.method)) {
        std::cerr << "Method not allowed: " << request.method << std::endl;
        sendErrorResponse(client_fd, 405, "Method not allowed");
        return;
    }
    
    // Procesar el request
    if (isCgiRequest(location, request.get_path())) {
        processCgiRequest(client_fd, request, location);
    } else {
        processStaticRequest(client_fd, server_config, location);
    }
}

ServerConfig* VirtualServersManager::findServerConfigForRequest(const HTTPRequest& request, ListenSocket* listen_socket) {
    std::string host = request.get_host();
    
    std::cout << "Looking for server config for host: '" << host << "'" << std::endl;
    std::cout << "Available virtual hosts: " << listen_socket->virtual_hosts.size() << std::endl;
    
    // Buscar coincidencia exacta de server_name
    for (std::vector<ServerConfig*>::iterator it = listen_socket->virtual_hosts.begin(); 
         it != listen_socket->virtual_hosts.end(); ++it) {
        
        ServerConfig* config = *it;
        std::cout << "Checking server config..." << std::endl;
        
        if (matchesServerName(config, host)) {
            std::cout << "Found matching server config for host: " << host << std::endl;
            return config;
        }
    }
    
    // Si no hay coincidencia, usar el primer servidor (default)
    std::cout << "No exact match found, using default server config" << std::endl;
    return getDefaultServerConfig(listen_socket);
}

bool VirtualServersManager::matchesServerName(const ServerConfig* config, const std::string& host) const {
    return config->matchesServerName(host);
}

ServerConfig* VirtualServersManager::getDefaultServerConfig(ListenSocket* listen_socket) const {
    if (listen_socket->virtual_hosts.empty()) {
        return NULL;
    }
    return listen_socket->virtual_hosts[0];  // El primer servidor es el default
}

Location* VirtualServersManager::findLocationForRequest(const HTTPRequest& request, const ServerConfig* server_config) {
    return server_config->findLocation(request.get_path());
}

bool VirtualServersManager::isMethodAllowed(const ServerConfig* server_config, const Location* location, HTTPMethod method) {
    std::string method_str;
    switch (method) {
        case GET: method_str = "GET"; break;
        case POST: method_str = "POST"; break;
        case PUT: method_str = "PUT"; break;
        case DELETE: method_str = "DELETE"; break;
        default: return false;
    }
    
    // Verificar primero en la location
    if (location && !location->getMethods().empty()) {
        const std::vector<std::string>& location_methods = location->getMethods();
        for (std::vector<std::string>::const_iterator it = location_methods.begin(); 
             it != location_methods.end(); ++it) {
            if (*it == method_str) {
                return true;
            }
        }
        return false;
    }
    
    // Si no está definido en location, usar configuración del servidor
    return server_config->isMethodAllowed(method_str);
}

// ================ RESPONSE HANDLING ================

void VirtualServersManager::sendErrorResponse(int client_fd, int status_code, const std::string& message) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " " << message << "\r\n"
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << message.length() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << message;
    
    std::string response_str = response.str();
    send(client_fd, response_str.c_str(), response_str.length(), 0);
}

void VirtualServersManager::sendErrorResponse(int client_fd, const HTTPError& error) {
    int status_code = static_cast<int>(error.status());
    sendErrorResponse(client_fd, status_code, error.msg());
}

bool VirtualServersManager::isCgiRequest(const Location* location, const std::string& path) {
    (void)location;
    return path.find(".cgi") != std::string::npos;
}

void VirtualServersManager::processCgiRequest(int client_fd, HTTPRequest& request, const Location* location) {
    (void)request;
    (void)location;
    
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 27\r\n"
        "Connection: close\r\n"
        "\r\n"
        "CGI processing placeholder";
    
    send(client_fd, response.c_str(), response.length(), 0);
}

void VirtualServersManager::processStaticRequest(int client_fd, const ServerConfig* server_config, const Location* location) {
    (void)server_config;
    (void)location;
    
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 45\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>Hello World!</h1></body></html>";
    
    send(client_fd, response.c_str(), response.length(), 0);
}

// ================ MAIN RUN METHOD ================

void VirtualServersManager::run() {
    std::cout << std::string(10, '=') << " Starting WEBSERVER " << std::string(10, '=') << std::endl;
    
    printVirtualHostsInfo();
    
    try {
        setupEpoll();
    } catch (const std::exception& e) {
        std::cerr << "Setup failed: " << e.what() << std::endl;
        return;
    }
    
    std::cout << std::string(10, '=') << " Starting EVENT LOOP " << std::string(10, '=') << std::endl;
    
    while (true) {
        int incoming = epoll_wait(_epoll_fd, _events.data(), _events.size(), -1);
        
        if (incoming < 0) {
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
            break;
        }
        
        for (int i = 0; i < incoming; ++i) {
            try {
                handleEvent(_events[i]);
            } catch (const std::exception& e) {
                std::cerr << "Error handling event: " << e.what() << std::endl;
            }
        }
    }
    
    std::cout << std::string(10, '=') << " Closing EVENT LOOP " << std::string(10, '=') << std::endl;
}

// ================ DEBUG METHODS ================

void VirtualServersManager::printVirtualHostsInfo() const {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "  VIRTUAL HOSTS CONFIGURATION" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    for (std::map<Listen, std::vector<ServerConfig*> >::const_iterator it = _virtual_hosts.begin(); 
         it != _virtual_hosts.end(); ++it) {
        
        std::cout << "\n📡 Listen Address: " << it->first.host << ":" << it->first.port << std::endl;
        std::cout << "   Virtual Hosts: " << it->second.size() << std::endl;
        
        for (size_t i = 0; i < it->second.size(); ++i) {
            const ServerConfig* config = it->second[i];
            std::cout << "   " << (i + 1) << ". ";
            
            if (i == 0) {
                std::cout << "[DEFAULT] ";
            }
            
            std::cout << "Server names: " << config->getServerNamesString();
            
            if (config->isDefaultServer()) {
                std::cout << " (catches all)";
            }
            
            std::cout << std::endl;
            std::cout << "      Root: " << config->getRoot() << std::endl;
            std::cout << "      Locations: " << config->getLocations().size() << std::endl;
        }
    }
    
    std::cout << std::string(50, '=') << std::endl;
}

void VirtualServersManager::printListenSocketsInfo() const {
    std::cout << "\n📊 Listen Sockets Information:" << std::endl;
    
    for (std::map<Listen, ListenSocket>::const_iterator it = _listen_sockets.begin(); 
         it != _listen_sockets.end(); ++it) {
        
        std::cout << "  🔌 " << it->first.host << ":" << it->first.port 
                  << " (FD: " << it->second.socket_fd << ")" 
                  << " - " << it->second.virtual_hosts.size() << " virtual hosts" << std::endl;
    }
}