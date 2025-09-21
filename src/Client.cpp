#include "Client.hpp"
#include "VirtualServersManager.hpp"

// Constructors, destructors
Client::Client(VirtualServersManager & vsm, int client_fd) // TODO no instance of overloaded function Client::Client matches the specified type
:_vsm(vsm)
, _cgi(_request, _vsm) 
, _element_parser(_error)
, _request_manager(_request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
, _response_manager(_cgi, _request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
, _status(PROCESSING_REQUEST)
, _socket(client_fd)
, _error_retry_count(0)
, _active_fd(client_fd, POLLIN | POLLRDHUP)
, _last_activity(time(NULL))
, _is_cgi(false)
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

	switch (_status)
	{
		case Client::PROCESSING_REQUEST: handle_processing_request(); break ;
		case Client::PROCESSING_RESPONSE: handle_processing_response(); break ;
		case Client::PROCESSING_CGI: handle_cgi_request(); break;
		case Client::CLOSING: CODE_ERR("Using CLOSE status"); break;
	}
}

// State initializers
void Client::prepareRequest()
{            
	_error_retry_count = 0;
	updateActiveFileDescriptor(_socket, POLLIN | POLLRDHUP);
	_request_manager.new_request();
	// Reset CGI.
	_status = PROCESSING_REQUEST;
}

void Client::prepareResponse(ServerConfig * server, Location * location, ResponseManager::RM_error_action action)
{
	_response_manager.new_response();
	_response_manager.set_location(location);
	_response_manager.set_virtual_server(server);
	_response_manager.generate_response(action, _is_cgi);

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
		Logger::getInstance() <<  "Request processed: " << _error.to_string() + ". " + _error.msg() << " Body size: " << _request.body.content.size() << std::endl;
		handleRequestDone();
	}
	else if (_error.status() != OK)
	{
		Logger::getInstance() <<  "Request processed with error: " << _error.to_string() + ". " + _error.msg() << std::endl;
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
		if (_request_manager.close())
		{
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

/*
	prepareCgi()
	{
		cgi.init();
		updateActiveFileDescriptor(cgi.get_active_file_descriptor());
		status = processing_CGI
	}
*/


void Client::handle_cgi_request() {	


	/*
		cgi.runCGI() // Read or write only once.
		if (cgi.done())
		{
			prepareResponse();
		}
		else if (cgi.error()? ) ? SI existe
		{
			handleRequestError();
		}
		else
		{
			updateActiveFileDescriptor(cgi.get_active_file_descriptor());
		}
	*/
	// cgi(_request, _vsm);

	_cgi.init(_request, _vsm);
	_cgi.runCGI();

	std::cout << "HERE IS THE CGI ANSWER: " << _cgi.getCGIResponse().getResponseBuffer() << std::endl;

	prepareResponse(NULL, NULL, ResponseManager::GENERATING_LOCATION_ERROR_PAGE);
}

void Client::handleRequestDone()
{
	ServerConfig * server_config = NULL;
	Location * location = NULL;
	get_config(&server_config, &location);

	if (!server_config)
		CODE_ERR("No server found for client " + wss::i_to_dec(_socket));
	else if (isCgiRequest()) {
			_status = PROCESSING_CGI; 
			// prepareCgi();
			handle_cgi_request();
	}
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

	if (err_page.empty())
		return prepareResponse(server_config, location, ResponseManager::GENERATING_DEFAULT_ERROR_PAGE);

    if (_request.method != HEAD)
	    _request.method = GET;
	_request.uri.path = err_page;
	location = server_config->findLocation(_request.get_path());
	prepareResponse(server_config, location, ResponseManager::GENERATING_LOCATION_ERROR_PAGE);
}

// Util, getters, setters, etc
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

bool Client::isCgiRequest()
{
	const std::string& path = _request.uri.path;

    for (t_cgi_conf::const_iterator it = CGIInterpreter::ACCEPTED_EXT.begin();
         it != CGIInterpreter::ACCEPTED_EXT.end(); ++it)
    {
        for (std::vector<std::string>::const_iterator ext = it->extensions.begin();
             ext != it->extensions.end(); ++ext)
        {
            if (path.size() >= ext->size() &&
                path.compare(path.size() - ext->size(), ext->size(), *ext) == 0)
            {
				_is_cgi = true;
                return (true);
            }
        }
    }
	_is_cgi = false;
    return (false);
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

			if (try_index[i].empty())
				continue;
	
			std::string new_request_path = _request.get_path() + try_index[i];

			// Get root path
			if (ptr_location)
				full_path = (*ptr_location)->getFilesystemLocation(new_request_path);
			if (full_path.empty() && !(*ptr_server_config)->getRoot().empty())
				full_path = (*ptr_server_config)->getRoot() + new_request_path;
    		else if (full_path.empty())
				CODE_ERR("No root path found");
	
			if (access(full_path.c_str(), F_OK) == 0) {

				Logger::getInstance() << "Index found: " + new_request_path << std::endl;
				
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