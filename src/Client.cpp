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
, _error_retry_count(0)
, _active_fd(client_fd, POLLIN | POLLRDHUP)
, _last_activity(time(NULL))
{
	_vsm.hookFileDescriptor(_active_fd);
}

Client::~Client() 
{
	if (_active_fd.fd >= 0)
		_vsm.unhookFileDescriptor(_active_fd);
	close(_socket);
}

// Entry point
void Client::process(int fd, int mode)
{
	_last_activity = time(NULL);

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
	Logger::getInstance() <<  "Processing on client: " + wss::i_to_dec(_socket) + " Done." << std::endl;
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
	if (0)
		Logger::getInstance() << _request << std::endl;
	
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
		Logger::getInstance() <<  "Request processed: " << _error.to_string() + ". " + _error.msg() << std::endl;
		handleRequestDone();
	}
	else if (_error.status() != OK)
	{
		Logger::getInstance() <<  "Request processed with error: " << _error.to_string() + ". " + _error.msg() << std::endl;
		// Logger::getInstance() << "Request: " << _request << std::endl;
		handleRequestError();
	}
}

void Client::handle_processing_response()
{
	// Check and handle error if exists. 
	_response_manager.process();
	if (_response_manager.is_error())
		handleRequestError();
	else if (_response_manager.response_done()) // Handle error
	{		
		// TODO
		/*  https://man7.org/linux/man-pages/man2/shutdown.2.html SHUT_RD
			Mientras no exista requestManager->close() usar error >= 400
		*/
		if (_request_manager.close())
		{
			// shutdown(_socket, SHUT_RD);
			_status = CLOSING;
			_vsm.unhookFileDescriptor(_active_fd);
			_active_fd.fd = -1;
		}
		else 
		{
			prepareRequest();
		}
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

	if (_error_retry_count == MAX_ERROR_RETRIES + 1 || ::status::status_type(_error.status()) == STYPE_EMPTY_ERROR_RESPONSE)
		return prepareResponse(server_config, location, ResponseManager::GENERATING_DEFAULT_ERROR_PAGE);
	else if (_error_retry_count > MAX_ERROR_RETRIES + 1)
		throw std::runtime_error("Too many internal redirects, can't resolve the request successfuly. Client " + wss::i_to_dec(_socket));

	std::string err_page = "";
	if (location)
		err_page = location->getErrorPage(_error.status());
	if (err_page.empty())
		err_page = server_config->getErrorPage(_error.status());

    if (_request.method != HEAD)
	    _request.method = GET;
	_request.uri.path = err_page;
	location = server_config->findLocation(_request.get_path());
	prepareResponse(server_config, location, ResponseManager::GENERATING_LOCATION_ERROR_PAGE);
}

// Util, getters, setters, etc
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

void Client::get_config(ServerConfig ** ptr_server_config, Location ** ptr_location)
{
	// Server
	*ptr_server_config = _vsm.findServerConfigForRequest(_request, _socket);

	// Location
	*ptr_location = (*ptr_server_config)->findLocation(_request.get_path());
	
	// Procesar index files solo para GET con request OK
	if ((_request.method == GET || _request.method == HEAD) && _error.status() == OK &&
		_request.get_path().at(_request.get_path().size() - 1) == '/')
	{
		std::string full_path;
		std::vector<std::string> try_index;

		// Get indexes vector
		if (*ptr_location && !(*ptr_location)->getIndex().empty()) {
			try_index = (*ptr_location)->getIndex();
		}
		else if (!(*ptr_server_config)->index_files.empty()) {
			try_index = (*ptr_server_config)->index_files; 
		}
		else {
			try_index.push_back("index.html");
		}

		for (size_t i = 0; i < try_index.size(); ++i) {

			if (try_index[i].empty()) {
				continue;
			}

			// _request.uri.path += try_index[i];
			
			// Eliminar posibles doble slash por la concatenacion "//"
			// bool root_end = root_path[root_path.size() - 1] == '/';
			// bool req_start = _request.get_path()[0] == '/';
			// bool req_end = _request.get_path()[_request.get_path().size() - 1] == '/';
			// bool try_index_start = try_index[i][0] == '/';


			// if (root_end && req_start) {
			// 	full_file_path = root_path + _request.get_path().substr(1);
			// }
			// else if (root_end || req_start) {
			// 	full_file_path = root_path + _request.get_path();
			// }
			// else {
			// 	full_file_path = root_path + "/" + _request.get_path();
			// }

			// if (req_end && try_index_start) {
			// 	full_file_path += try_index[i].substr(1);
			// }
			// else if (req_end || try_index_start) {
			// 	full_file_path += try_index[i];
			// }
			// else {
			// 	full_file_path += "/" + try_index[i];
			// }
	
			std::string new_request_path = _request.get_path() + try_index[i];

			// Get root path
			if (ptr_location)
				full_path = (*ptr_location)->getFilesystemLocation(new_request_path);
			if (full_path.empty() && !(*ptr_server_config)->getRoot().empty())
				full_path = (*ptr_server_config)->getRoot() + new_request_path;
    		else if (full_path.empty())
				CODE_ERR("No root path found");
	
			Logger::getInstance() << "Trying index: " + new_request_path << " Full: " << full_path << std::endl;

			if (access(full_path.c_str(), F_OK) == 0) {

				Logger::getInstance() << "Found: " + new_request_path << std::endl;
				
				// Guardar index encontrado
				_request.uri.path = new_request_path;
				*ptr_location = (*ptr_server_config)->findLocation(_request.get_path());
/*				Para un location con configuración para el archivo index en sí 
				Location* new_loc = (*ptr_server_config)->findLocation(_request.get_path());
				if (new_loc) {
				}
*/
				break;
			}
		}
	}
}

bool Client::closing() const
{
	return _status == CLOSING;
}
