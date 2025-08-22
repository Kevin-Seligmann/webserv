#include "VirtualServersManager.hpp"
#include "StringUtil.hpp"
#include "Logger.hpp"
#include "DebugView.hpp"

// ================ STATIC MEMBER ================

std::map<int, VirtualServersManager::ClientState*> VirtualServersManager::ClientState::client_states;

// ================ CLIENT STATE IMPLEMENTATION ================

VirtualServersManager::ClientState::ClientState(int client_fd)
    : client_fd(client_fd)
    , element_parser(error)
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

void VirtualServersManager::ClientState::changeStatus(Status new_status, const std::string& reason) {
    Status old_status = status;
    status = new_status;

    std::ostringstream oss;
    oss << "Chanfe Client Status from " << old_status << " to " << status << " for " << client_fd << " because of: " << reason; 
    Logger::getInstance().info(oss.str());;
}

std::string VirtualServersManager::ClientState::statusToString(Status s) {
    switch(s) {
        case READING_REQUEST: return "READING_REQUEST";
        case PROCESSING_REQUEST: return "PROCESSING_REQUEST";
        case ERROR_HANDLING: return "ERROR_HANDLING";
        case WRITING_RESPONSE: return "WRITING_RESPONSE";
        case WAITING_FILE: return "WAITING_FILE";
        case WAITING_CGI: return "WAITING_CGI";
        case CLOSING: return "CLOSING";
        case CLOSED: return "CLOSED";
        default: return "UNKNOWN";
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

    Logger::getInstance().info("Initializing VirtualServersManager with " + wss::i_to_dec(configs.size()) + " server configurations");
    _server_configs.reserve(configs.size() * 2);
    // TODO: agregar parse de allow_upload en ParsedServer, es necesario?
    // Procesar cada configuración parseada (C++98 compatible)
    for (std::vector<ParsedServer>::const_iterator parsed_it = configs.begin(); 
         parsed_it != configs.end(); ++parsed_it) {
        
        const ParsedServer& parsed_server = *parsed_it;
        
        Logger::getInstance().info("  Processing server with " + wss::i_to_dec(parsed_server.listens.size()) + " listen directives");
        
        // Crear ServerConfig a partir de ParsedServer
        ServerConfig server_config(parsed_server);
        _server_configs.push_back(server_config);
        
        // Obtener puntero al ServerConfig recién agregado (C++98 safe)
        ServerConfig* config_ptr = &_server_configs[_server_configs.size() - 1];
        
        // Procesar todas las directivas listen de este servidor
        for (std::vector<Listen>::const_iterator listen_it = parsed_server.listens.begin(); 
             listen_it != parsed_server.listens.end(); ++listen_it) {
            
            const Listen& listen_config = *listen_it;
            
            Logger::getInstance().info("    Mapping to " + listen_config.host + ":" + wss::i_to_dec(listen_config.port));
            
            // Agregar al mapeo de virtual hosts
            _virtual_hosts[listen_config].push_back(config_ptr);
            
            // Crear ListenSocket si no existe (C++98 compatible)
            if (_listen_sockets.find(listen_config) == _listen_sockets.end()) {
                ListenSocket new_socket(listen_config);
                _listen_sockets[listen_config] = new_socket;
                
                Logger::getInstance().info("    Created new ListenSocket for " + listen_config.host + ":" + wss::i_to_dec(listen_config.port));
            }
            
            // Agregar config al ListenSocket QUESTION: esta listo?
            std::map<Listen, ListenSocket>::iterator socket_it = _listen_sockets.find(listen_config);
            if (socket_it != _listen_sockets.end()) {
                socket_it->second.virtual_hosts.push_back(config_ptr);
            }
        }
    }
    
    Logger::getInstance().info("Virtual hosts mapping completed:");
    Logger::getInstance().info("  Total ServerConfigs: " + wss::i_to_dec(_server_configs.size()));
    Logger::getInstance().info("  Total Listen addresses: " + wss::i_to_dec(_listen_sockets.size()));
    Logger::getInstance().info("  Total Virtual host groups: " + wss::i_to_dec(_virtual_hosts.size()));
}

VirtualServersManager::~VirtualServersManager() {
    Logger::getInstance().info("Destroying VirtualServersManager...");
    
    // Limpiar map de client_fd to listen_socket
    _client_to_listen_socket.clear();

    // Cerrar epoll
    if (_epoll_fd >= 0) {
        close(_epoll_fd);
        Logger::getInstance().info("  Closed epoll fd: " + wss::i_to_dec(_epoll_fd));
    }
    
    // Cerrar todos los sockets de escucha
    for (std::map<Listen, ListenSocket>::iterator it = _listen_sockets.begin(); 
         it != _listen_sockets.end(); ++it) {
        
        if (it->second.socket_fd >= 0) {
            close(it->second.socket_fd);
            Logger::getInstance().info("  Closed socket fd: " + wss::i_to_dec(it->second.socket_fd) + " for " + it->first.host + ":" + wss::i_to_dec(it->first.port));
        }
    }
    
    // Limpiar estados de clientes
    for (std::map<int, ClientState*>::iterator client_it = ClientState::client_states.begin();
         client_it != ClientState::client_states.end(); ++client_it) {
        delete client_it->second;
    }
    ClientState::client_states.clear();
    
    Logger::getInstance().info("VirtualServersManager destroyed.");
}

// ================ SOCKET MANAGEMENT ================

void VirtualServersManager::createListenSockets() {
    Logger::getInstance().info("Creating listen sockets...");
    
    for (std::map<Listen, ListenSocket>::iterator it = _listen_sockets.begin(); 
         it != _listen_sockets.end(); ++it) {
        
        Logger::getInstance().info("Setting up socket for " + it->first.host + ":" + wss::i_to_dec(it->first.port));
        
        try {
            setupListenSocket(it->second);
            bindListenSocket(it->second);
            
            Logger::getInstance().info("Socket " + wss::i_to_dec(it->second.socket_fd) + " created for " + it->first.host + ":" + wss::i_to_dec(it->first.port) + " with " + wss::i_to_dec(it->second.virtual_hosts.size()) + " virtual hosts");
                      
        } catch (const std::exception& e) {
            std::ostringstream oss;
            oss << e.what() << " for " << it->first.host << ":" << it->first.port;
            Logger::getInstance().error(oss.str());
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
    Logger::getInstance().info("Setting up epoll...");
    
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
    
    Logger::getInstance().info("Epoll configured with " + wss::i_to_dec(_listen_sockets.size()) + " listen sockets");
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
    
    Logger::getInstance().info("New client connected on FD: " + wss::i_to_dec(client_fd) + " to " + listen_socket->listen_config.host + ":" + wss::i_to_dec(listen_socket->listen_config.port));
    
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
        switch (client->status) {
            case ClientState::READING_REQUEST:
                handleReadingRequest(client_fd, client);
                break;
            case ClientState::PROCESSING_REQUEST:
                handleProcessingRequest(client_fd, client);
                break;
            case ClientState::ERROR_HANDLING:
                handleErrorHandling(client_fd, client);
                break;
            case ClientState::WRITING_RESPONSE:
                handleWritingResponse(client_fd, client);
                break;
            case ClientState::WAITING_FILE:
                handleWaitingFile(client_fd, client);
                break;
            case ClientState::WAITING_CGI:
                handleWaitingCGI(client_fd, client);
                break;
            case ClientState::CLOSING:
                handleClosing(client_fd, client);
                break;
            case ClientState::CLOSED:
                handleClosed(client_fd, client);
                break;
        }
    
    } catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "Exception processing client data: " << e.what();
        Logger::getInstance().error(oss.str());
        disconnectClient(client_fd);
    }
}

void VirtualServersManager::handleReadingRequest(int client_fd, ClientState* client) {
    Logger::getInstance().info("Reading request for client " + wss::i_to_dec(client_fd));

    try {   
        client->request_manager.process();
        
        if (client->hasError()) {
            Logger::getInstance().warning("Error parsing request: " + client->error.to_string());

            if (client->error_retry_count == 0) {
                client->original_request = client->request;
            }

            client->status = ClientState::ERROR_HANDLING;
            client->error_retry_count = 0;
            return;
        }
        
        if (client->isRequestComplete()) {
            Logger::getInstance().info("Request parsing complete for client " + wss::i_to_dec(client_fd));
            client->status = ClientState::PROCESSING_REQUEST;
            return;
        }
    } catch (const std::exception &e) {
        Logger::getInstance().error("Exception in handleReadingRequest: " + std::string(e.what()));
        client->error.set("Internal error during request reading", INTERNAL_SERVER_ERROR);
        client->status = ClientState::ERROR_HANDLING;
    }
}

void VirtualServersManager::handleProcessingRequest(int client_fd, ClientState* client) {
    Logger::getInstance().info("Processing request for client " + wss::i_to_dec(client_fd));

    try {
        std::map<int, ListenSocket*>::iterator map_it = _client_to_listen_socket.find(client_fd);
        if (map_it == _client_to_listen_socket.end()) {
            client->error.set("No listen socket found for client", INTERNAL_SERVER_ERROR);
            client->status = ClientState::ERROR_HANDLING;
            return;
        }

        ListenSocket* listen_socket = map_it->second;
        ServerConfig* server_config = findServerConfigForRequest(client->request, listen_socket);
        if (!server_config) {
            client->error.set("Virtual host not found", NOT_FOUND);
            client->status = ClientState::ERROR_HANDLING;
            return;
        }

        // DEBUG PARA ENCONTRAR EL ERROR DE ERRO 500

        // Antes de buscar location
        Logger::getInstance().error("DEBE DAR: 'PATH_DE_LOCATION' ");
        Logger::getInstance().info("Searching location for path: '" + client->request.get_path() + "'");

        Logger::getInstance().info("Available locations in server config:");
        std::map<std::string, Location>::const_iterator l_it = server_config->locations.begin();
        for (;l_it != server_config->locations.end(); ++l_it) {
            Logger::getInstance().info("  - '" + l_it->first + "' (match_type: " + 
            std::string(l_it->second.getMatchType() == Location::EXACT ? "EXACT" : "PREFIX") + ")");
        }

        Location* location2 = server_config->findLocation(client->request.get_path());
        if (!location2) {
            Logger::getInstance().error("CRITICAL: No location found for path '" + client->request.get_path() + "'");
        }

        // FIN DEBUG PARA ERROR DE ERROR 500

        Location* location = server_config->findLocation(client->request.get_path());
        if (!location) {
            location = server_config->findLocation("/", false);
            if (!location) {
                client->error.set("No location configured", INTERNAL_SERVER_ERROR);
                client->status = ClientState::ERROR_HANDLING;
                return;
            }
        }

        Logger::getInstance().error(location2->getPath() == location->getPath() ? "\n\nSON IGUALES" : "\n\nsNO SON IGUALES");

        if (!isMethodAllowed(server_config, location, client->request.method)) {
            client->error.set("Method not allowed", METHOD_NOT_ALLOWED);
            client->status = ClientState::ERROR_HANDLING;
            return;
        }

        client->response_manager.set_virtual_server(server_config);
        client->response_manager.set_location(location);

        if (isCgiRequest(location, client->request.get_path())) {
            Logger::getInstance().info("CGI request detected");
            client->status = ClientState::WAITING_CGI;
            return;
        }

        // Generar respuesta (puede ser normal o error)
        client->response_manager.generate_response();
        ResponseManager::RM_status rm_status = client->response_manager.get_status();

        // Miramos si el ResponseManager nos pide operar sobre un archivo
        ActiveFileDescriptor afd = client->response_manager.get_active_file_descriptor();
        if (afd.fd != -1) {
            Logger::getInstance().info("Registering file FD " + wss::i_to_dec(afd.fd));
            struct epoll_event ev;
            ev.data.fd = afd.fd;
            ev.events = afd.mode;
            if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, afd.fd, &ev) == -1) {
                Logger::getInstance().error("epoll_ctl ADD failed for file fd: " + std::string(strerror(errno)));
            }
            client->status = ClientState::WAITING_FILE;
            return;
        }

        // Si no hay archivo pendiente, miramos si toca escribir al socket
        if (rm_status == ResponseManager::WRITING_RESPONSE) {
            Logger::getInstance().info("Registering client FD " + wss::i_to_dec(client_fd) + " for EPOLLOUT");
            struct epoll_event ev;
            ev.data.fd = client_fd;
            ev.events = EPOLLOUT;
            if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
                Logger::getInstance().error("epoll_ctl MOD failed for client fd: " + std::string(strerror(errno)));
            }
            client->status = ClientState::WRITING_RESPONSE;
            return;
        }

        // Caso por defecto: cerramos con error
        Logger::getInstance().warning("Unexpected ResponseManager status after generate_response");
        client->status = ClientState::ERROR_HANDLING;

    } catch (std::exception &e) {
        std::ostringstream oss;
        oss << "Exception in handleProcessingRequest: " << std::string(e.what());
        Logger::getInstance().error(oss.str());
        client->error.set("Internal error during request processing", INTERNAL_SERVER_ERROR);
        client->status = ClientState::ERROR_HANDLING;
    }   
}

void VirtualServersManager::handleErrorHandling(int client_fd, ClientState* client) {
    Logger::getInstance().info("Handling error for client " + wss::i_to_dec(client_fd) + 
                              ": " + client->error.to_string());

    try {
        // No loops infinitos con la misma request
        if (client->error_retry_count >= MAX_ERROR_RETRIES) {
            Logger::getInstance().warning("Max error retries reached, sending default error page");
            client->response_manager.generate_response();
            client->status = ClientState::CLOSING;
            disconnectClient(client_fd);
            // QUESTION es necesario desconectar el fd? y si es keep alive? en HTTP1.1 se deja abierta o se cierra?
            return;
        }

        // Intentar reescribir la request para error page custom
        if (attemptErrorPageRewrite(client_fd, client)) {
            Logger::getInstance().info("Error page rewrite successful");
            client->status = ClientState::PROCESSING_REQUEST;
            client->error_retry_count++;
            return;
        }

        Logger::getInstance().info("No custom error page, switching to default error page");
        client->response_manager.generate_response();
        
        if (client->error.status() >= 400) {
            client->status = ClientState::CLOSING;
            disconnectClient(client_fd);
        } else {
            client->status = ClientState::WRITING_RESPONSE;
        }

    } catch (const std::exception& e) {
        Logger::getInstance().error("Exception in handleErrorHandling: " + std::string(e.what()));
        client->error.set("Critical error in error handling", INTERNAL_SERVER_ERROR);
        client->response_manager.generate_response();
        client->status = ClientState::CLOSING;
    }
}

void VirtualServersManager::handleWritingResponse(int client_fd, ClientState* client) {
    Logger::getInstance().info("Writing response for client " + wss::i_to_dec(client_fd));
    
    try {
        client->response_manager.process(); // QUESTION donde mas se usa, que hace? Checkear
        
        if (client->response_manager.response_done()) {
            Logger::getInstance().info("Response sending complete for client " + wss::i_to_dec(client_fd));
            
            if (client->request.headers.close_status == RCS_CLOSE) { // QUESTION que codigo es este, que otros hay, su contexto, usos de su familia, checkear
                client->status = ClientState::CLOSING;
            } else {
                // Keep-alive: preparar para nueva request
                client->request_manager.new_request(); // QUESTION que hace, checkear
                client->response_manager.reset(); // Acceso público a new_request()
                client->error.set("", OK);
                client->error_retry_count = 0;
                client->status = ClientState::READING_REQUEST; // QUESTION que pasa si en la siguiente iteracion del bucle principal no hay datos en el socket?
            }
            return;
        }

    Logger::getInstance().info("Continue writing response");
    
    } catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "Exception in handleWritingResponse: " << e.what();
        Logger::getInstance().error(oss.str());
        client->status = ClientState::CLOSING;
    }
}

void VirtualServersManager::handleWaitingFile(int client_fd, ClientState* client) {
    Logger::getInstance().info("Processing file I/O for client " + wss::i_to_dec(client_fd));
    
    try {
        ActiveFileDescriptor afd = client->response_manager.get_active_file_descriptor(); // QUESTION que es ActiveFileDescripto, contexto checkear
        
        if (afd.fd == -1) {
            Logger::getInstance().warning("No active file descriptor, switching to response");
            client->status = ClientState::WRITING_RESPONSE;
            return;
        }

        client->response_manager.process();
        
        if (client->response_manager.get_active_file_descriptor().fd == -1) {
            Logger::getInstance().info("File I/O done, preparing to send response");
            client->status = ClientState::WRITING_RESPONSE;
            return;
        }

        Logger::getInstance().info("Continue with writng/reading file");
        
    } catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "Exception in handleWaitingFile: " << e.what();
        Logger::getInstance().error(oss.str());
        client->error.set("File I/O error", INTERNAL_SERVER_ERROR);
        client->status = ClientState::ERROR_HANDLING;
    }
}

void VirtualServersManager::handleWaitingCGI(int client_fd, ClientState* client) {  // QUESTION explicarlo todo
    Logger::getInstance().info("Handling CGI for client " + wss::i_to_dec(client_fd));
    
    try {
        static std::map<int, time_t> cgi_start_times;
        
        if (cgi_start_times.find(client_fd) == cgi_start_times.end()) {
            cgi_start_times[client_fd] = time(NULL);
            Logger::getInstance().info("Starting CGI execution...");
            return;
        }
        
        time_t elapsed = time(NULL) - cgi_start_times[client_fd];
        if (elapsed > 30) { // timeout de 30 segundos
            Logger::getInstance().warning("CGI timeout");
            cgi_start_times.erase(client_fd);
            client->error.set("CGI script timeout", INTERNAL_SERVER_ERROR);
            client->status = ClientState::ERROR_HANDLING;
            return;
        }
        
        if (elapsed >= 1) { // 1 o más segundos de procesamiento //QUESTION porque damos por completo cgi con 1 segundo de procesamiento
            Logger::getInstance().info("CGI execution complete");
            cgi_start_times.erase(client_fd);
            
            client->status = ClientState::WRITING_RESPONSE;
            return;
        }
        
        Logger::getInstance().info("Continue waiting for CGI");
        
    } catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "Exception in handleWaitingCGI: " << e.what();
        Logger::getInstance().error(oss.str());
        client->error.set("CGI execution error", INTERNAL_SERVER_ERROR);
        client->status = ClientState::ERROR_HANDLING;
    }
} // ILYA como va el cgi-executor?

void VirtualServersManager::handleClosing(int client_fd, ClientState* client) {
    Logger::getInstance().info("Closing connection for client " + wss::i_to_dec(client_fd));
    
    try {
        if (client->response_manager.get_active_file_descriptor().fd != -1) {
            // TODO: Implementar cleanup en ResponseManager
            // KEVIN hay este meteodo de cleanup de ResponseManager?
            // QUESTION existe un cleanup para RequestManager? Donde se lo llama si existe?
        }
        
        std::string final_status = client->hasError() ? "ERROR: " + client->error.to_string() : "SUCCESS";
        Logger::getInstance().info("Client " + wss::i_to_dec(client_fd) + " session ended: " + final_status);
        
        client->status = ClientState::CLOSED;
        
    } catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "Exception in handleClosing: " << e.what();
        Logger::getInstance().error(oss.str());
        client->status = ClientState::CLOSED;
    }
}

void VirtualServersManager::handleClosed(int client_fd, ClientState* client) {
    (void)client;
    Logger::getInstance().info("Client " + wss::i_to_dec(client_fd) + " is closed");
    disconnectClient(client_fd);
}


// ================ METHODS AUX DE ERROR ================

bool VirtualServersManager::attemptErrorPageRewrite(int client_fd, ClientState* client) { // QUESTION explicar
    std::map<int, ListenSocket*>::iterator map_it = _client_to_listen_socket.find(client_fd);
    if (map_it == _client_to_listen_socket.end()) {
        return false;
    }

    ListenSocket* listen_socket = map_it->second;
    ServerConfig* server_config = findServerConfigForRequest(client->request, listen_socket);
    Location* location = NULL;
    
    if (server_config) {
        location = server_config->findLocation(client->request.get_path());
    }

    // Buscar error page
    int status_code = static_cast<int>(client->error.status());
    std::string error_page_path;
    
    if (server_config) {
        // buscqueda jerarquizada loc > serv > def
        error_page_path = server_config->getErrorPage(status_code, location);
    }
    
    if (error_page_path.empty()) {
        return false; // No hay custom error page
    }

    // Backup original request
    if (client->error_retry_count == 0) {
        client->original_request = client->request;
    }
    
    // Crear nueva request GET para la error page
    client->request.reset();
    client->request.method = GET;
    client->request.protocol = "HTTP/1.1";
    client->request.uri.path = error_page_path;
    client->request.headers.host = client->original_request.headers.host;
    client->request.headers.port = client->original_request.headers.port;
    
    // Headers para debugging
    client->request.headers.put("X-Original-URI", client->original_request.uri.path);
    client->request.headers.put("X-Error-Status", status::stoa(client->error.status()));
    
    // Reset error
    client->error.set("", OK);
    
    Logger::getInstance().info("Request rewritten for error page: " + error_page_path);
    return true;
}

// ================ DISCONECT FD ================

void VirtualServersManager::disconnectClient(int client_fd) {
    Logger::getInstance().info("Disconnecting client FD: " + wss::i_to_dec(client_fd));
    
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
    Logger::getInstance().info("Processing complete request for FD: " + wss::i_to_dec(client_fd));
    Logger::getInstance().info("Host: " + request.get_host() + ", Path: " + request.get_path());
    
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
    
    Logger::getInstance().info("Selected server config for host: " + request.get_host());
    
    // Encontrar la location apropiada
    Location* location = server_config->findLocation(request.get_path());
    if (!location) {
        std::cerr << "No location found for path " << request.get_path() << std::endl;
        sendErrorResponse(client_fd, 404, "Location not found");
        return;
    }
    
    // Verificar si el método está permitido
    if (!isMethodAllowed(server_config, location, request.method)) {
        std::cerr << "Method not allowed" << std::endl;
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
    
    Logger::getInstance().info("Looking for server config for host: '" + host + "'");
    Logger::getInstance().info("Available virtual hosts: " + wss::i_to_dec(listen_socket->virtual_hosts.size()));
    
    // Buscar coincidencia exacta de server_name
    for (std::vector<ServerConfig*>::iterator it = listen_socket->virtual_hosts.begin(); 
         it != listen_socket->virtual_hosts.end(); ++it) {
        
        ServerConfig* config = *it;
        Logger::getInstance().info("Checking server config...");
        
        if (matchesServerName(config, host)) {
            Logger::getInstance().info("Found matching server config for host: " + host);
            return config;
        }
    }
    
    // Si no hay coincidencia, usar el primer servidor (default)
    Logger::getInstance().info("No exact match found, using default server config");
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
    if (!location) {
        return false;
    }

    std::string cgi_extension = location->getCgiExtension(); // QUESTION bien implementado?
    if (cgi_extension.empty()) {
        return false;
    }
    
    return path.size() >= cgi_extension.size() &&
           path.compare(path.size() - cgi_extension.size(), cgi_extension.size(), cgi_extension) == 0; // QUESTION como va compare?
}

void VirtualServersManager::processCgiRequest(int client_fd, HTTPRequest& request, const Location* location) { // ILYA implementar aqui
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
    Logger::getInstance().info(std::string(10, '=') + " Starting WEBSERVER " + std::string(10, '='));
    // Use DebugView boxed format
    {
        std::vector<std::string> lines;
        for (std::map<Listen, std::vector<ServerConfig*> >::const_iterator it = _virtual_hosts.begin();
             it != _virtual_hosts.end(); ++it) {
            lines.push_back("listen " + it->first.host + ":" + wss::i_to_dec(it->first.port) + " | vhosts: " + wss::i_to_dec(it->second.size()));
            for (size_t i = 0; i < it->second.size(); ++i) {
                const ServerConfig* cfg = it->second[i];
                std::string tag = (i == 0) ? " [DEFAULT]" : "";
                lines.push_back("  - " + wss::i_to_dec(i + 1) + tag + " names: " + cfg->getServerNamesString());
                lines.push_back("     root: " + cfg->getRoot() + " | locations: " + wss::i_to_dec(cfg->locations.size()));
            }
        }
        DebugView::printBox("VIRTUAL HOSTS", lines);
    }
    
    try {
        setupEpoll();
    } catch (const std::exception& e) {
        std::cerr << "Setup failed: " << e.what() << std::endl;
        return;
    }
    
    Logger::getInstance().info(std::string(10, '=') + " Starting EVENT LOOP " + std::string(10, '='));
    
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
    
    Logger::getInstance().info(std::string(10, '=') + " Closing EVENT LOOP " + std::string(10, '='));
}

// ================ DEBUG METHODS ================

void VirtualServersManager::printVirtualHostsInfo() const {
    std::vector<std::string> lines;
    for (std::map<Listen, std::vector<ServerConfig*> >::const_iterator it = _virtual_hosts.begin();
         it != _virtual_hosts.end(); ++it) {
        lines.push_back("listen " + it->first.host + ":" + wss::i_to_dec(it->first.port) + " | vhosts: " + wss::i_to_dec(it->second.size()));
        for (size_t i = 0; i < it->second.size(); ++i) {
            const ServerConfig* cfg = it->second[i];
            std::string tag = (i == 0) ? " [DEFAULT]" : "";
            lines.push_back("  - " + wss::i_to_dec(i + 1) + tag + " names: " + cfg->getServerNamesString());
            lines.push_back("     root: " + cfg->getRoot() + " | locations: " + wss::i_to_dec(cfg->locations.size()));
        }
    }
    DebugView::printBox("VIRTUAL HOSTS", lines);
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