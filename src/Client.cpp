#include "Client.hpp"
#include "VirtualServersManager.hpp"

Client::Client(VirtualServersManager & vsm, std::vector<ServerConfig> & servers, int client_fd)
:_vsm(vsm) 
, _servers(servers)
, _element_parser(_error)
, _request_manager(_request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
, _response_manager(_request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
, _status(PROCESING_REQUEST)
, _socket(client_fd)
, _active_fd(client_fd, EPOLLIN | EPOLLRDHUP)
{
    _vsm.hookFileDescriptor(_active_fd);
}

#include  <cstdio>
Client::~Client() 
{
    _vsm.unhookFileDescriptor(_active_fd);
    close(_socket);
}

void Client::process(int fd, int mode)
{
    if (fd != _active_fd.fd && !(_active_fd.mode & mode))
        CODE_ERR("Trying to access client with an invalid socket or mode");

    Logger::getInstance() <<  "Processing on client: " + wss::i_to_dec(_socket) + ". Mode: " + wss::i_to_dec(mode) << std::endl;

    switch (_status)
    {
        case Client::PROCESING_REQUEST: request(); break ;
        case Client::PROCESING_RESPONSE: response(); break ;
        default: break ;
    }
}

void Client::request()
{
    _request_manager.process();
    if (_request_manager.has_error())
        Logger::getInstance() <<  "Error parsing request: " << _error.to_string() << std::endl;
    else if (_request_manager.request_done())
        Logger::getInstance() <<  "Request parsed successfuly: " << _error.to_string() << std::endl;
    
    if (_request_manager.has_error() || _request_manager.request_done())
    {
        // Check and handle error if exists. 

	    ServerConfig* target_server = findServerForRequest(_request);
        if (!target_server)
            _error.set("Server match not found", NOT_FOUND);

        Location* location = findLocationForRequest(_request, target_server);
        if (!location)
            _error.set("Can't match request URI. Path: " + _request.get_path(), NOT_FOUND);

        // if (isCgiRequest(location, _request.get_path())) {
        //     processCgiRequest(location);
        // }
        // else {
            prepareResponse(target_server, location);
        // }
    }
}

void Client::response()
{
    // Check and handle error if exists. 


    _response_manager.process();
    if (_response_manager.response_done()) // Handle error
    {
        prepareRequest();
    }
    else 
    {
        updateActiveFileDescriptor(_response_manager.get_active_file_descriptor());
    }
}

void Client::prepareRequest()
{
    updateActiveFileDescriptor(_socket, EPOLLIN | EPOLLRDHUP);
    _request_manager.new_request();
    _status = PROCESING_REQUEST;
}

void Client::prepareResponse(ServerConfig * server, Location * location)
{
    _response_manager.set_location(location);
    _response_manager.set_virtual_server(server);
    _response_manager.generate_response();

    updateActiveFileDescriptor(_response_manager.get_active_file_descriptor());
    _status = PROCESING_RESPONSE;
}

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

ServerConfig* Client::findServerForRequest(const HTTPRequest& request) {
	// Implement match based on Host header and port
	(void)request;
	if (!_servers.empty()) {
		return &_servers[0];
	}
	return NULL;
}

Location* Client::findLocationForRequest(const HTTPRequest& request, const ServerConfig* server) {
    // Busca la Location adecuada para la request en el server dado
    if (!server)
        return NULL;
    ServerConfig* no_const_server = const_cast<Server*>(server);
    ServerConfig::ConfigLayer* config = &no_const_server->getConfig();
    Location* location_found = config->findLocation(request.get_path());
    return const_cast<Location*>(location_found);
}

bool Client::isCgiRequest(Location* location, const std::string& path) {
	// Implement CGI detection based on location configuration
	(void)location;
	(void)path;
	return path.find(".cgi") != std::string::npos; // true si uri de la request termina en cgi
}

void Client::processCgiRequest(Location* location) {
	// Implement CGI request processing
	(void)location;
	
	std::string response = 
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 27\r\n"
		"\r\n"
		"CGI processing placeholder";
	
	send(_socket, response.c_str(), response.length(), 0);
}

int Client::getSocket() const {return _socket;}

int Client::ownsFd(int fd) const {return fd == _socket || fd == _active_fd.fd;}