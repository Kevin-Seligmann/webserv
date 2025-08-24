#include "Client.hpp"
#include "VirtualServersManager.hpp"


// Entry point
void Client::process(int fd, int mode)
{
    if (fd != _active_fd.fd && !(_active_fd.mode & mode))
        CODE_ERR("Trying to access client with an invalid socket or mode");

    Logger::getInstance() <<  "Processing on client: " + wss::i_to_dec(_socket) + ". Mode: " + wss::i_to_dec(mode) << std::endl;

    switch (_status)
    {
        case Client::PROCESSING_REQUEST: handle_processing_request(); break ;
        case Client::PROCESSING_RESPONSE: handle_processing_response(); break ;
        case Client::PROCESSING_CGI: handle_cgi_request(); break;
        // case Client::CLOSING: handle_closing(client_fd, client); break;
    }
}

// State initializers
void Client::prepareRequest()
{            
    _error_retry_count = 0;
    updateActiveFileDescriptor(_socket, EPOLLIN | EPOLLRDHUP);
    _request_manager.new_request();
    _status = PROCESSING_REQUEST;
}

void Client::prepareResponse(ServerConfig * server, Location * location)
{
    _response_manager.set_location(location);
    _response_manager.set_virtual_server(server);
    _response_manager.generate_response();

    updateActiveFileDescriptor(_response_manager.get_active_file_descriptor());
    _status = PROCESSING_RESPONSE;
}

// State handlers
void Client::handle_processing_request()
{
    // void VirtualServersManager::handleReadingRequest(int client_fd, ClientState* client) {        
    //         if (client->hasError()) {

    //             if (client->error_retry_count == 0) {
    //                 client->original_request = client->request;
    //             }

    //             client->status = ClientState::ERROR_HANDLING;
    //             client->error_retry_count = 0;
    //             return;
    //         }
    //     }
    // }

    _request_manager.process();

    // Should be more specific about which errors require looking at Location
    if (_error.status() != OK)
    {
        Logger::getInstance() <<  "Request processed with error: " << _error.to_string() + ": " + _error.msg() << std::endl;
    }
    else if (_request_manager.request_done())
    {
        Logger::getInstance() <<  "Request processed: " << _error.to_string() + ": " + _error.msg() << std::endl;
        ServerConfig * server_config;
        Location * location;
        get_config(&server_config, &location);

        if (!server_config)
            _error.set("Couldn't find a server for this request", NOT_FOUND);
        else if (!location)
            _error.set("Couldn't find a location for this request. Path: " + _request.get_path(), NOT_FOUND);

        // else if (isCgiRequest(location, _request.get_path())) {
        //     prepareCgiResponse(target_server, location);
        // }
        else {
            prepareResponse(server_config, location);
        }
    }
}

void Client::handle_processing_response()
{
    // Check and handle error if exists. 
    _response_manager.process();
    if (_response_manager.response_done()) // Handle error
    {
        // QUESTION que codigo es este, que otros hay, su contexto, usos de su familia, checkear
        /*
            Si la request "obliga" a cerrar la conexión por algun motivo. Por ejemplo Connection:close.
        */

        // if (client->request.headers.close_status == RCS_CLOSE)
        //     client->status = ClientState::CLOSING;
        // else
            prepareRequest();
    }
    else 
    {
        updateActiveFileDescriptor(_response_manager.get_active_file_descriptor());
    }
}

void Client::handle_cgi_request() {
	// Implement CGI request processing

//         static std::map<int, time_t> cgi_start_times;
        
//         if (cgi_start_times.find(client_fd) == cgi_start_times.end()) {
//             cgi_start_times[client_fd] = time(NULL);
//             Logger::getInstance().info("Starting CGI execution...");
//             return;
//         }
        
//         time_t elapsed = time(NULL) - cgi_start_times[client_fd];
//         if (elapsed > 30) { // timeout de 30 segundos
//             Logger::getInstance().warning("CGI timeout");
//             cgi_start_times.erase(client_fd);
//             client->error.set("CGI script timeout", INTERNAL_SERVER_ERROR);
//             client->status = ClientState::ERROR_HANDLING;
//             return;
//         }
        
//         if (elapsed >= 1) { // 1 o más segundos de procesamiento //QUESTION porque damos por completo cgi con 1 segundo de procesamiento
//             Logger::getInstance().info("CGI execution complete");
//             cgi_start_times.erase(client_fd);
            
//             client->status = ClientState::WRITING_RESPONSE;
//             return;
//         }
	
	std::string response = 
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 27\r\n"
		"\r\n"
		"CGI processing placeholder";
	
	send(_socket, response.c_str(), response.length(), 0);
}

void Client::handle_closing()
{

}

void Client::process_status_error() 
{
    _error_retry_count ++;

    /*
        If this happens it could be an error trying to write the request. It's not 
        possible to reliably send a response, the connection should be closed.
    */
    if (_error_retry_count >= MAX_ERROR_RETRIES)
    {
        // _status = CLOSING;
        // How to handle this?
        // QUESTION es necesario desconectar el fd? y si es keep alive? en HTTP1.1 se deja abierta o se cierra?
        return ;
    }

    _response_manager.generate_response();
    _status = PROCESSING_RESPONSE;

//         // Intentar reescribir la request para error page custom
//         if (attemptErrorPageRewrite(client_fd, client)) {
//             Logger::getInstance().info("Error page rewrite successful");
//             client->status = ClientState::PROCESSING_REQUEST;
//             client->error_retry_count++;
//             return;
//         }

//         Logger::getInstance().info("No custom error page, switching to default error page");
//         client->response_manager.generate_response();
        
//         if (client->error.status() >= 400) {
//             client->status = ClientState::CLOSING;
//             disconnectClient(client_fd);
//         } else {
//             client->status = ClientState::WRITING_RESPONSE;
//         }

// bool VirtualServersManager::attemptErrorPageRewrite(int client_fd, ClientState* client) { // QUESTION explicar
//     std::map<int, ListenSocket*>::iterator map_it = _client_to_listen_socket.find(client_fd);
//     if (map_it == _client_to_listen_socket.end()) {
//         return false;
//     }

//     ListenSocket* listen_socket = map_it->second;
//     ServerConfig* server_config = findServerConfigForRequest(client->request, listen_socket);
//     Location* location = NULL;
    
//     if (server_config) {
//         location = server_config->findLocation(client->request.get_path());
//     }

//     // Buscar error page
//     int status_code = static_cast<int>(client->error.status());
//     std::string error_page_path;
    
//     if (server_config) {
//         // buscqueda jerarquizada loc > serv > def
//         error_page_path = server_config->getErrorPage(status_code, location);
//     }
    
//     if (error_page_path.empty()) {
//         return false; // No hay custom error page
//     }

//     // Backup original request
//     if (client->error_retry_count == 0) {
//         client->original_request = client->request;
//     }
    
//     // Crear nueva request GET para la error page
//     client->request.reset();
//     client->request.method = GET;
//     client->request.protocol = "HTTP/1.1";
//     client->request.uri.path = error_page_path;
//     client->request.headers.host = client->original_request.headers.host;
//     client->request.headers.port = client->original_request.headers.port;
    
//     // Headers para debugging
//     client->request.headers.put("X-Original-URI", client->original_request.uri.path);
//     client->request.headers.put("X-Error-Status", status::stoa(client->error.status()));
    
//     // Reset error
//     client->error.set("", OK);
    
//     Logger::getInstance().info("Request rewritten for error page: " + error_page_path);
//     return true;
// }


}

    
                
// } // ILYA como va el cgi-executor?


// State endings

// Transitions


// Constructors, destructors
Client::Client(VirtualServersManager & vsm, int client_fd)
:_vsm(vsm) 
, _element_parser(_error)
, _request_manager(_request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
, _response_manager(_request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
, _status(PROCESSING_REQUEST)
, _socket(client_fd)
, _active_fd(client_fd, EPOLLIN | EPOLLRDHUP)
{
    _vsm.hookFileDescriptor(_active_fd);
}

Client::~Client() 
{
    _vsm.unhookFileDescriptor(_active_fd);
    close(_socket);
}

// Util, getters, setters, etc

void Client::changeStatus(Status new_status, const std::string& reason) {
    Status old_status = _status;
    _status = new_status;
    Logger::getInstance() << "Change Client Status from " << old_status << " to " << _status << " for " << _socket << " because of: " << reason; 
}

std::string Client::statusToString(Status s) {
    switch(s) {
        case PROCESSING_CGI: return "PROCESSING_CGI";
        case PROCESSING_REQUEST: return "PROCESSING_REQUEST";
        case PROCESSING_RESPONSE: return "PROCESSING_RESPONSE";
        case CLOSING: return "CLOSING";
    }
    CODE_ERR("Imposible status");
}

int Client::getSocket() const {return _socket;}

int Client::ownsFd(int fd) const {return fd == _socket || fd == _active_fd.fd;}

void Client::updateActiveFileDescriptor(int fd, int mode)
{
    ActiveFileDescriptor newfd(fd, mode);
    if (newfd == _active_fd)
        return ;
    _vsm.swapFileDescriptor(_active_fd, newfd);
    _active_fd = newfd;
}

void Client::updateActiveFileDescriptor(ActiveFileDescriptor newfd)
{
    if (newfd == _active_fd)
        return ;
    _vsm.swapFileDescriptor(_active_fd, newfd);
    _active_fd = newfd;
}

bool Client::isCgiRequest(Location* location, const std::string& path) {
	// Implement CGI detection based on location configuration
	(void)location;
	(void)path;
	return path.find(".cgi") != std::string::npos; // true si uri de la request termina en cgi
}

// void VirtualServersManager::get_config(ServerConfig ** ptr_server_config, Location ** ptr_location)
// {
// 	std::map<int, ListenSocket*>::iterator map_it = _client_to_listen_socket.find(client_fd);
// 	if (map_it == _client_to_listen_socket.end()) {
// 		*ptr_server_config = NULL;
// 		*ptr_location = NULL;
// 		return;
// 	}

// 	ListenSocket* listen_socket = map_it->second;
// 	ServerConfig* server_config = findServerConfigForRequest(client->request, listen_socket);
// 	if (!server_config) {
// 		*ptr_server_config = NULL;
// 		*ptr_location = NULL;
// 		return;
// 	}

// 	Location* location = server_config->findLocation(client->request.get_path());
// 	// What happens if there are no locations.
// 	if (!location)
// 		location = server_config->findLocation("/", false);
// 	*ptr_server_config = server_config;
// 	*ptr_location = location; 
// }