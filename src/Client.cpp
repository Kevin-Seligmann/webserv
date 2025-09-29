#include "Client.hpp"
#include "VirtualServersManager.hpp"

// Constructors, destructors
Client::Client(VirtualServersManager & vsm, int client_fd) // TODO no instance of overloaded function Client::Client matches the specified type
: _vsm(vsm)
, _status(IDLE)
, _error()
, _request()
, _cgi()
, _element_parser(_error)
, _request_manager(_request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
, _response_manager(_cgi, _request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
, _socket(client_fd)
, _error_retry_count(0)
, _last_activity(time(NULL))
, _is_cgi(false)
{
	_vsm.hookFileDescriptor(ActiveFileDescriptor(client_fd, POLLIN | POLLRDHUP));
	_active_fds.push_back(ActiveFileDescriptor(client_fd, POLLIN | POLLRDHUP));
}

Client::~Client() 
{
	for (std::vector<ActiveFileDescriptor>::iterator it = _active_fds.begin(); it != _active_fds.end(); it ++)
		_vsm.unhookFileDescriptor(*it);
	_active_fds.clear();
	close(_socket);
} 

// Entry point
void Client::process(int fd)
{
	/*
		Sometimes a file descriptor can lag in the poll even if we are not interested anymore.
		This snippet makes sure we don't execute anything we don't want.
	*/
	bool found = false;
	for (std::vector<ActiveFileDescriptor>::iterator it = _active_fds.begin(); it != _active_fds.end(); it ++)
	{
		if (fd == it->fd)
		{
			found = true;
			break ;
		}
	}
	if (!found)
		return ;


	_last_activity = time(NULL);
	switch (_status)
	{
		case Client::IDLE: setStatus(PROCESSING_REQUEST, "Processing Request");  
		case Client::PROCESSING_REQUEST: handle_processing_request(); break ;
		case Client::PROCESSING_RESPONSE: handle_processing_response(); break ;
		case Client::PROCESSING_CGI: handle_cgi_request(fd); break;
		case Client::CLOSING: CODE_ERR("Using CLOSE status"); break;
	}
}

// State initializers
void Client::prepareRequest()
{            
	_error_retry_count = 0;
	updateActiveFileDescriptor(_socket, POLLIN | POLLRDHUP);
	_request_manager.new_request();
	_cgi.reset();
	setStatus(IDLE, "Idle");
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
		setStatus(PROCESSING_RESPONSE, "Processing Request");
	}
}

void Client::prepareCgi()
{
	setStatus(PROCESSING_CGI, "Processing CGI");

	ServerConfig * _server_config = NULL;
	Location * _location = NULL;
	get_config(&_server_config, &_location);
	std::string path = "";
    if (_location)
        path = _location->getFilesystemLocation(_request.get_path());
    if (path.empty() && !_server_config->getRoot().empty())
        path = _server_config->getRoot() + _request.get_path();
	else if (path.empty())
		CODE_ERR("No root path found for " + _request.get_path() + " Server: " + _server_config->getRoot());

	_cgi.init(_request, _vsm, path, _server_config, _location);

	if (_cgi.error())
	{
		_error.set("Error processing CGI", INTERNAL_SERVER_ERROR);
		handleRequestError();
	}
	else 
	{
		updateActiveFileDescriptors(_cgi.getActiveFileDescriptors());
	}
};

// State handlers
void Client::handle_processing_request()
{
	_request_manager.process();

	if (_request_manager.request_done() && _error.status() == OK)
	{
		Logger::getInstance() <<  "Request processed: " << _error.to_string() + _error.msg() << " Body size: " << _request.body.content.size() << std::endl;
		handleRequestDone();
	}
	else if (_error.status() != OK)
	{
		Logger::getInstance() <<  "Request processed with error: " << _error.to_string() + _error.msg() << std::endl;
		handleRequestError();
	}
}

void Client::handle_processing_response()
{
	// Check and handle error if exists. 
//	DEBUG_LOG(" ++++++++ REQUEST PROCESS ++++++++++ ");
	_response_manager.process();
	if (_response_manager.is_error()) {
		handleRequestError();
	}
	else if (_response_manager.response_done())
	{		
		// TODO: Closing
		
		// if (_request_manager.close())
		// {
		// 	setStatus(CLOSING, "Closing");
		// 	updateActiveFileDescriptors(std::vector<ActiveFileDescriptor>());
		// }
		// else 
		// {
			prepareRequest();
		// }
	}
	else 
	{
//		DEBUG_LOG(" QUE HAY AQUI " << _request.get_path());
//		DEBUG_LOG(" ESTO ES LA REQUEST : " << _request);
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


void Client::handle_cgi_request(int fd) 
{	
	_cgi.runCGI(fd);
	if (_cgi.done())
	{
		prepareResponse(NULL, NULL, ResponseManager::GENERATING_LOCATION_ERROR_PAGE);
		_cgi.reset();
		_request.reset();
	}
	else if (_cgi.error())
	{
		_error.set("CGI Fatal error", INTERNAL_SERVER_ERROR);
		handleRequestError();
	}
	else 
	{
		updateActiveFileDescriptors(_cgi.getActiveFileDescriptors());
	}
}

void Client::handleRequestDone()
{
	ServerConfig * server_config = NULL;
	Location * location = NULL;
	get_config(&server_config, &location);

	size_t max_size = 0;
	if (location && location->getMaxBodySize() > 0)
		max_size = location->getMaxBodySize();
	else
		max_size = server_config->getClientMaxBodySize();

	if (max_size < _request.body.content.length())
    {
        _error.set("Body size too large: " + wss::i_to_dec(max_size) + " \\ " + wss::i_to_dec(_request.body.content.length()), CONTENT_TOO_LARGE);
		handleRequestError();
		return ;
    }

	if (!server_config)
		CODE_ERR("No server found for client " + wss::i_to_dec(_socket));
	else if (isCgiRequest())
		prepareCgi();
	else 
		prepareResponse(server_config, location, ResponseManager::GENERATING_LOCATION_ERROR_PAGE);
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
bool Client::isCgiRequest()
{
//	DEBUG_LOG(" >>>>>>>>>> YES IT'S CGI REQUEST " << _request.uri.path);

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

void Client::setStatus(Status status, std::string const & txt)
{
	_status = status;
	Logger::getInstance() << "Client " << _socket << " set status " << (int) status << " " << txt << std::endl;
}

bool Client::idle() const
{
	return _status == IDLE;
}



// FD Management
int Client::ownsFd(int fd) const
{
	for (std::vector<ActiveFileDescriptor>::const_iterator it = _active_fds.begin(); it != _active_fds.end(); it ++)
		if (it->fd == fd)
			return true;
	return false;
}

void Client::updateActiveFileDescriptor(int fd, int mode)
{
	updateActiveFileDescriptor(ActiveFileDescriptor(fd, mode));
}

void Client::updateActiveFileDescriptor(ActiveFileDescriptor newfd)
{
	bool hooked = false;

	for (std::vector<ActiveFileDescriptor>::const_iterator it = _active_fds.begin(); it != _active_fds.end(); it ++)
	{
		if (it->fd == newfd.fd && it->mode == newfd.mode)
		{
			hooked = true;

		}
		else if (it->fd == newfd.fd)
		{
			hooked = true;
			_vsm.updateFiledescriptor(newfd);
		}
		else 
			_vsm.unhookFileDescriptor(*it);
	}
	if (!hooked)
		_vsm.hookFileDescriptor(newfd);

	_active_fds.clear();
	_active_fds.push_back(newfd);
}

void Client::updateActiveFileDescriptors(std::vector<ActiveFileDescriptor> newfds)
{
    for (std::vector<ActiveFileDescriptor>::const_iterator old_it = _active_fds.begin(); old_it != _active_fds.end(); old_it ++)
    {
        bool still_needed = false;
        for (std::vector<ActiveFileDescriptor>::const_iterator new_it = newfds.begin(); new_it != newfds.end(); new_it ++)
        {
            if (old_it->fd == new_it->fd)
            {
                still_needed = true;
                break;
            }
        }
        if (!still_needed)
            _vsm.unhookFileDescriptor(*old_it);
    }

    for (std::vector<ActiveFileDescriptor>::const_iterator new_it = newfds.begin(); new_it != newfds.end(); new_it ++)
    {
        bool exists = false;
        for (std::vector<ActiveFileDescriptor>::const_iterator old_it = _active_fds.begin(); old_it != _active_fds.end(); old_it ++)
        {
            if (old_it->fd == new_it->fd)
            {
                exists = true;
                if (old_it->mode != new_it->mode)
                    _vsm.updateFiledescriptor(*new_it);
                break;
            }
        }
        if (!exists)
            _vsm.hookFileDescriptor(*new_it);
    }
    _active_fds = newfds;
}
