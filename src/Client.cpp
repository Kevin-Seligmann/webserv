#include "Client.hpp"
#include "VirtualServersManager.hpp"

// Constructors, destructors
Client::Client(VirtualServersManager & vsm, int client_fd)
:_vsm(vsm) 
, _element_parser(_error)
, _request_manager(_request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
, _response_manager(_request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
, _status(PROCESSING_REQUEST)
, _socket(client_fd)
, _active_fd(client_fd, POLLIN | POLLRDHUP)
, _last_activity(time(NULL))
{
	_vsm.hookFileDescriptor(_active_fd);
}

Client::~Client() 
{
	_vsm.unhookFileDescriptor(_active_fd);
	close(_socket);
}

// Entry point
void Client::process(int fd, int mode)
{
	if (fd != _active_fd.fd || !(_active_fd.mode & mode)) {
		CODE_ERR("Trying to access client with an invalid socket or mode");
	}

	switch (_status)
	{
		case Client::PROCESSING_REQUEST:
			handle_processing_request();
			break;
		case Client::PROCESSING_RESPONSE:
			handle_processing_response();
			break;
		case Client::PROCESSING_CGI:
			handle_cgi_request();
			break;
		case Client::CLOSING:
			handle_closing();
			break;
	}
}

// State initializers
void Client::prepareRequest()
{            
	_error_retry_count = 0;
	updateActiveFileDescriptor(_socket, POLLIN | POLLRDHUP);
	_request_manager.new_request();
	_status = PROCESSING_REQUEST;
}

void Client::prepareResponse(ServerConfig * server, Location * location, ResponseManager::RM_error_action action)
{
	_response_manager.new_response();
	_response_manager.set_location(location);
	_response_manager.set_virtual_server(server);
	_response_manager.generate_response(action);

	if(_response_manager.is_error())
	{
		handleRequestError();
	}
	else 
	{
		updateActiveFileDescriptor(_response_manager.get_active_file_descriptor());
		_status = PROCESSING_RESPONSE;
	}
}

// State handlers
void Client::handle_processing_request()
{
	_request_manager.process();

	if (_request_manager.request_done() && _error.status() == OK)
	{
		handleRequestDone();
	}
	else if (_error.status() != OK)
	{
		handleRequestError();
	}
 }

void Client::handle_processing_response()
{
	if (_response_manager.response_done())
	{	
		_last_activity = time(NULL);

		if (_request.headers.close_status == RCS_CLOSE ||
			_error.status() == INTERNAL_SERVER_ERROR ||
			_error.status() == NOT_IMPLEMENTED ||
			_error.status() == VERSION_NOT_SUPPORTED ||
			(_request.protocol == "HTTP/1.0" && _request.headers.close_status != RCS_KEEP_ALIVE))
		{	
			shutdown(_socket, SHUT_RD);
			_status = CLOSING;
		}
		else
		{
			prepareRequest();
			updateActiveFileDescriptor(_socket, POLLIN | POLLRDHUP);
		}
	}
	else
	{
		_last_activity = time(NULL);
		_response_manager.process();
		updateActiveFileDescriptor(_response_manager.get_active_file_descriptor());
	}
}

void Client::handle_closing() {
	sleep(CLOSING_TIMEOUT);
	if (time(NULL) - _last_activity > CLOSING_TIMEOUT)
		_vsm.disconnectClient(_socket); 

	if (_response_manager.response_done()) {
		_vsm.disconnectClient(_socket);
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

// State transitions
void Client::handleRequestDone()
{
	ServerConfig * server_config = NULL;
	Location * location = NULL;
	get_config(&server_config, &location);

	if (!server_config)
		CODE_ERR("No server found for client " + wss::i_to_dec(_socket));
	/* TO_DELETE ? UNCOMMENT */
	// else if (isCgiRequest(location, _request.get_path())) {
	//     prepareCgiResponse(target_server, location);
	// }
	else {
		prepareResponse(server_config, location, ResponseManager::GENERATING_LOCATION_ERROR_PAGE);
	}
}

void Client::handleRequestError() 
{
	Logger::getInstance() << "Handling request error. Retry count " << _error_retry_count << std::endl;
	_error_retry_count ++;

	ServerConfig * server_config = NULL;
	Location * location = NULL;
	get_config(&server_config, &location);

	// Custom error page
	if (_error_retry_count == 1) { // ::status::status_type(_error.status()) == STYPE_EMPTY_ERROR_RESPONSE)

		std::string error_page = "";

		if (location)
			error_page = location->getErrorPage(_error.status());

		if (error_page.empty() && server_config)
			error_page = server_config->getErrorPage(_error.status());

		if (!error_page.empty()) {
			if (_request.method != HEAD)
				_request.method = GET;
			_request.uri.path = error_page;
			location = server_config->findLocation(_request.get_path());
			prepareResponse(server_config, location, ResponseManager::GENERATING_LOCATION_ERROR_PAGE);
			return;
		}
	}
	prepareResponse(server_config, location, ResponseManager::GENERATING_DEFAULT_ERROR_PAGE);

}

// Util, getters, setters, etc

void Client::updateActiveFileDescriptor(int fd, int mode)
{
	// Prevenir cambios durante el procesamiento de request
    if (_status == PROCESSING_REQUEST && fd != _socket)
    {
        Logger::getInstance().error("Attempted to change FD during request processing!");
        return;
    }

	ActiveFileDescriptor newfd(fd, mode);
	if (newfd == _active_fd)
		return ;
	_vsm.swapFileDescriptor(_active_fd, newfd);
	_active_fd = newfd;
}

void Client::updateActiveFileDescriptor(ActiveFileDescriptor newfd)
{    
	if (newfd == _active_fd) {
		Logger::getInstance() << "No change needed" << std::endl;
		return ;
	}
	_vsm.swapFileDescriptor(_active_fd, newfd);
	_active_fd = newfd;
}

bool Client::isKeepAlive() const {
	if (_request.protocol == "HTTP/1.1") {
		return _request.headers.close_status != RCS_CLOSE;
	}
	return false;
}

bool Client::isCgiRequest(Location* location, const std::string& path) {
	// Implement CGI detection based on location configuration
	(void)location;
	(void)path;
	return path.find(".cgi") != std::string::npos; // true si uri de la request termina en cgi
}

void Client::get_config(ServerConfig ** ptr_server_config, Location ** ptr_location)
{
    static std::map<int, int> client_resolve_count;
 
	// Get server
	*ptr_server_config = _vsm.findServerConfigForRequest(_request, _socket);
	if (!*ptr_server_config) {
		CODE_ERR("No server found for client " + wss::i_to_dec(_socket));
	}

	std::string final_path;
	*ptr_location = (*ptr_server_config)->resolveRequest(_request.get_path(), final_path);

	if (final_path != _request.get_path()) {
		_request.uri.path = final_path;
	}
}