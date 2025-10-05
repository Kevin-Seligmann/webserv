#include "Client.hpp"
#include "VirtualServersManager.hpp"

// Constructors, destructors
Client::Client(VirtualServersManager & vsm, int client_fd) // TODO no instance of overloaded function Client::Client matches the specified type
: _vsm(vsm)
, _status(IDLE)
, _error()
, _request()
, _cgi(_stream_request)
, _element_parser(_error)
, _request_manager(_request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd, _stream_request)
, _response_manager(_cgi, _request, _error, SysBufferFactory::SYSBUFF_SOCKET, client_fd, _stream_request)
, _socket(client_fd)
, _error_retry_count(0)
, _last_activity(time(NULL))
, _is_cgi(false)
, _previous_directory_path("")
, _max_size(1048576)
{
	_vsm.hookFileDescriptor(ActiveFileDescriptor(client_fd, POLLIN | POLLRDHUP));
	_active_fds.push_back(ActiveFileDescriptor(client_fd, POLLIN | POLLRDHUP));
}

Client::~Client() 
{
	for (std::vector<ActiveFileDescriptor>::iterator it = _active_fds.begin(); it != _active_fds.end(); it ++)
		_vsm.unhookFileDescriptor(*it);
	close(_socket);
} 

// Entry point
void Client::process(int fd, int mode)
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
	if (!found || _status == CLOSING)
		return ;


	_last_activity = time(NULL);
	switch (_status)
	{
		case Client::IDLE: setStatus(PROCESSING_REQUEST, "Processing Request");  
		case Client::PROCESSING_REQUEST: handle_processing_request(); break ;
		case Client::PROCESSING_RESPONSE: handle_processing_response(); break ;
		case Client::PROCESSING_CGI: handle_cgi_request(fd); break;
		case Client::STREAMING: process_stream(fd, mode); break ;
		case Client::CLOSING: CODE_ERR("Using CLOSE status"); break;
	}
}

// State initializers
void Client::prepareRequest()
{            
	_error_retry_count = 0;
	updateActiveFileDescriptor(_socket, POLLIN | POLLRDHUP);
	_request_manager.new_request();
	_response_manager.new_response(false);
	_cgi.reset();
	// no resetear _previous_directory_path, tiene que persistir entre requests consecutivos
	setStatus(IDLE, "Idle");
}

void Client::prepareResponse(ServerConfig * server, Location * location, ResponseManager::RM_error_action action)
{
	bool preserve = (_error_retry_count > 0 && _error.status() == MOVED_PERMANENTLY);
	_response_manager.new_response(preserve);
	_response_manager.set_location(location);
	_response_manager.set_virtual_server(server);
	_response_manager.generate_response(action, _is_cgi && _error.status() == OK);

	if(_response_manager.is_error())
	{
		handleRequestError();
	}
	else 
	{
		updateActiveFileDescriptor(_response_manager.get_active_file_descriptor());
		setStatus(PROCESSING_RESPONSE, "Processing response");
	}
}

bool Client::probablyAutoindex() const
{
	/*
		Sin _previousdirectory_path es false
		Si el directorio actual empieza con el previo, es posible
		Si solo hay 1 nivel de profundidad de diferencia es muy posible
	*/

	if (_previous_directory_path.empty())
	{
		return false;
	}

	std::string current = _request.uri.path;
	size_t prev_dir_len = _previous_directory_path.length();


	if (current.compare(0, prev_dir_len, _previous_directory_path) != 0)
	{
		return false;
	}

	if (current.length() <= prev_dir_len)
	{
		return false;
	}

	std::string added_path = current.substr(prev_dir_len);

	return (added_path.find('/') == std::string::npos);
}

void Client::prepareCgi(ServerConfig* server, Location* location)
{
	setStatus(PROCESSING_CGI, "Processing CGI");

	std::string path = "";

	// para extraer el path sin path_info
	std::string request_path = _request.uri.path;
	std::string script_path = request_path;

	// buscar fin del script
	for (t_cgi_conf::const_iterator it = CGIInterpreter::ACCEPTED_EXT.begin();
		it != CGIInterpreter::ACCEPTED_EXT.end(); ++it)
	{
		for (std::vector<std::string>::const_iterator ext = it->extensions.begin();
			 ext != it->extensions.end(); ++ext)
		{
			size_t pos = request_path.find(*ext);
			if (pos != std::string::npos)
			{
				script_path = request_path.substr(0, pos + ext->size());
				break;
			}
		}
	}

	// construir path físico solo del script
    if (location)
	{
        path = location->getFilesystemLocation(script_path);
	}

    if (path.empty() && !server->getRoot().empty())
	{
        path = server->getRoot() + script_path;
	}
	else if (path.empty())
	{
		CODE_ERR("No root path found for " + _request.get_path() + " Server: " + server->getRoot());
	}

	DEBUG_LOG("CGI physical path: " << path);

	_cgi.init(_request, _vsm, path, server, location);

	if (_cgi.error())
	{
		_error.set("Error processing CGI", INTERNAL_SERVER_ERROR, true);
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
	if (_stream_request.streaming_active)
	{
		Logger::getInstance() <<  "Streaming Request started: " << _error.to_string() + _error.msg() << std::endl;
		handleRequestDone();
	}
	else if (_request_manager.request_done() && _error.status() == OK)
	{
		DEBUG_LOG("\n\n");
		DEBUG_LOG("Request processed: " << _error.to_string() + _error.msg() << " Body size: " << _request.body.content.size());
		handleRequestDone();
	}
	else if (_error.status() != OK)
	{
		DEBUG_LOG("Request processed with error: " << (_error.to_string() + _error.msg()));
		handleRequestError();
	}
}

void Client::handle_processing_response()
{
	_response_manager.process();

	if (_response_manager.is_error())
	{
		handleRequestError();
	}
	else if (_response_manager.response_done())
	{
		if (_error.status() == OK && !_request.uri.path.empty()
			&& _request.uri.path[_request.uri.path.length() - 1] == '/') // Es probable que sea autoindex, se guarda
		{
			_previous_directory_path = _request.uri.path;
			DEBUG_LOG("Client"<< _socket
					  << ": Posible autoindex directory: "
					   << _previous_directory_path);
		}
		else if (_error.status() == OK && !probablyAutoindex())
		{
			_previous_directory_path = "";
		}
	
		if (_request_manager.close())
		{
		 	setStatus(CLOSING, "Closing");
//		 	updateActiveFileDescriptors(std::vector<ActiveFileDescriptor>());
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
		_error.set("CGI Fatal error", INTERNAL_SERVER_ERROR, true);
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

	if (!server_config)
	{
		CODE_ERR("No server found for client " + wss::i_to_dec(_socket));
	}
	
	// Conseguir el _max_size de location > server > fallback por constructor = 1M
	if (location && location->getMaxBodySize() >= 0)
		_max_size = location->getMaxBodySize();
	else
		_max_size = server_config->getClientMaxBodySize();

	DEBUG_LOG("HandleRequestDone MAX SIZE = " << _max_size << " at file: '"
			  << __FILE__ << "' - line: " << __LINE__);
	DEBUG_LOG("HandleRequestDone REQUEST SIZE = " << _request.body.content.length() << " at file: '"
			  << __FILE__ << "' - line: " << __LINE__);

	size_t headers_len = _request.body.content.find("\r\n\r\n");
	size_t body_len = _request.body.content.length() - headers_len;

	DEBUG_LOG("HandleRequestDone BODY LEN = " << body_len << " at file: '"
			  << __FILE__ << "' - line: " << __LINE__);


	if (_max_size < body_len) // TODO editado
    {
        _error.set("Body size exceeds limit: " + wss::i_to_dec(_request.body.content.length())
					+ " bytes (max: " + wss::i_to_dec(_max_size) + ")", CONTENT_TOO_LARGE);
		handleRequestError();
		return ;
    }

	if (_stream_request.streaming_active && _error.status() == OK)
		prepareRequestStreaming();
	else if (!probablyAutoindex() && isCgiRequest(location))
	{
		prepareCgi(server_config, location); // AUTOINDEX .PY BEHAVIOUR ORDER
	}
	else 
		prepareResponse(server_config, location, ResponseManager::GENERATING_LOCATION_ERROR_PAGE);
}

void Client::handleRequestError() 
{
	_is_cgi = false;
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
bool Client::isCgiRequest(Location* location)
{
/* 	ServerConfig* server_config = NULL;
	Location* location = NULL;
	get_config(&server_config, &location); */

	if (!location || location->getCgiExtension().empty())
	{
		_is_cgi = false;
		return false;
	}

	const std::string& path = _request.uri.path;

    for (t_cgi_conf::const_iterator it = CGIInterpreter::ACCEPTED_EXT.begin();
         it != CGIInterpreter::ACCEPTED_EXT.end(); ++it)
    {
        for (std::vector<std::string>::const_iterator ext = it->extensions.begin();
             ext != it->extensions.end(); ++ext)
        {
			// hay que buscar .py o .php en cualquier uicacion del path no solo al final
            size_t pos = path.find(*ext);
			if (pos != std::string::npos)
			{
				// verificar que despues de la extension hay '/' o nada
				size_t after = pos + ext->size();
				if (after == path.size() || path[after] == '/')
				{
					_is_cgi = true;
					return (true);
				}
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
	
	// redirect manejado
	if (*ptr_location && !(*ptr_location)->getRedirect().empty())
	{
		_error.set("Redirect configured", MOVED_PERMANENTLY);
		// asignar _redirecting_location
		return;
	}

	// alias manejado en Location::getFilesystemLocation()

	// index files manejado para GET/HEAD con request OK
	if ((_request.method == GET || _request.method == HEAD) && _error.status() == OK)
		// && _request.get_path().at(_request.get_path().size() - 1) == '/')
	{
		std::vector<std::string> try_index;

		// Get index files o fallback 
		if (*ptr_location && !(*ptr_location)->getIndex().empty())
		{
			try_index = (*ptr_location)->getIndex();
		}
		else if (!(*ptr_server_config)->index_files.empty())
		{
			try_index = (*ptr_server_config)->getIndexFiles();
		}
		else {
			try_index.push_back("index.html");
		}

		for (size_t i = 0; i < try_index.size(); ++i) {

			if (try_index[i].empty())
				continue;
	
			std::string new_request_path = normalizePath(_request.get_path(), try_index[i]);

			DEBUG_LOG(">>> Valor de new_request_path : " + new_request_path);

			std::string full_path;
			// Get root path
			if (ptr_location)
			{
				full_path = (*ptr_location)->getFilesystemLocation(new_request_path);
				// DEBUG_LOG(">>> Valor de full_path con location : " + full_path);
			}
			if (full_path.empty() && !(*ptr_server_config)->getRoot().empty())
			{
				full_path = (*ptr_server_config)->getRoot() + new_request_path;
				// DEBUG_LOG(">>> Valor de full_path con server : " + full_path);

			}
    		else if (full_path.empty())
			{
				CODE_ERR("No root path found");
			}
	
			if (access(full_path.c_str(), F_OK) == 0)
			{
				Logger::getInstance() << ">>> Index found: " + new_request_path << std::endl;
				
				// Guardar index encontrado
				_request.uri.path = new_request_path;

				if (!ptr_location || (*ptr_location)->getMatchType() != Location::EXACT)
				{
					*ptr_location = (*ptr_server_config)->findLocation(_request.get_path());
				}
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
	{
		if (it->fd == fd)
			return true;
	}
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
    // for (std::vector<ActiveFileDescriptor>::const_iterator old_it = newfds.begin(); old_it != newfds.end(); old_it ++)
	// 	Logger::getInstance() << "Putting fd: " << old_it->fd << " Pollin: " << (old_it->mode & POLLIN) << " pollout " << (old_it->mode & POLLOUT) << std::endl;

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

void Client::prepareRequestStreaming()
{
	if (!wss::isCgiRequest(_request.get_path()))
	{
		_error.set("Internal redirect from streaming to non streaming request", INTERNAL_SERVER_ERROR, true);
		handleRequestError();
		return ;
	}
	setStatus(STREAMING, "Streaming");

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

	_cgi.initStreamed(_request, _vsm, path, _server_config, _location);
	if (_cgi.error())
	{
		_error.set("Error processing CGI", INTERNAL_SERVER_ERROR, true);
		handleRequestError();
		return ;
	}
	else 
	{
		updateStreamingFileDescriptors();
	}
}

void Client::process_stream(int fd, int mode)
{
	// std::cout << " REMAINING BUFFER: " << _request_manager.get_buffer_remaining_size() << std::endl;
	if (fd == _socket)
	{
		if (mode & POLLIN && !_stream_request.request_read_finished)
		{
			_request_manager.process();
			if (_error.status() != OK)
			{
				Logger::getInstance() <<  "Request processed with error: " << _error.to_string() + _error.msg() << std::endl;
				handleRequestError();
				return ;
			}
			
			DEBUG_LOG("ProcessStream VALOR DE: '_max_size': " << _max_size);
			DEBUG_LOG("ProcessSrtram VALOR DE: '_stream_request.request_body_size': " << _stream_request.request_body_size);

			if (_max_size < _stream_request.request_body_size)
			{
				_error.set("Body size too large: " + wss::i_to_dec(_max_size) + " \\ "
							+ wss::i_to_dec(_stream_request.request_body_size), CONTENT_TOO_LARGE, true);
				handleRequestError();
				return ;
			}

			if (_request_manager.request_done())
			{
				_stream_request.request_read_finished = true;
				Logger::getInstance() << "Client " << _socket << " finished request read streaming " << std::endl;
			}
		}
		if (mode & POLLOUT && !_stream_request.cgi_write_finished)
		{
			_cgi.sendResponse();
			if (_error.status() != OK)
			{
				Logger::getInstance() <<  "Request processed with error: " << _error.to_string() + _error.msg() << std::endl;
				handleRequestError();
				return ;
			}
		}
		updateStreamingFileDescriptors();
	}
	else if (!_cgi.done() && (fd == _cgi.write_fd() || fd == _cgi.read_fd()))
	{
		_cgi.runCGIStreamed(fd);
		if (_cgi.error())
		{
			_error.set("CGI Fatal error", INTERNAL_SERVER_ERROR, true);
			handleRequestError();
			return ;
		}
		updateStreamingFileDescriptors();
	}

	// 
	bool should_close = _cgi.getCGIResponse().close || _request_manager.close();
	// Only after both are complete. Reading request must finish in order to empty buffer and continue with the connection.
	if (_stream_request.cgi_write_finished && (_stream_request.request_read_finished || should_close))
	{
		if (should_close)
		{
			setStatus(CLOSING, "Closing");
//			shutdown(_socket, SHUT_RD);
			// updateActiveFileDescriptors(std::vector<ActiveFileDescriptor>());
		}
		else 
		{
			prepareRequest();
		}
	}
}

void Client::updateStreamingFileDescriptors()
{
	std::vector<ActiveFileDescriptor> fds;

	int socket_flag = 0;
	if (_stream_request.streaming_active && !_request_manager.request_done())
		socket_flag |= POLLIN;
	if (!_stream_request.cgi_write_finished)
		socket_flag |= POLLOUT;
	if (socket_flag != 0)
		fds.push_back(ActiveFileDescriptor(_socket, socket_flag | POLLRDHUP));

	std::vector<ActiveFileDescriptor> cgi_fds = _cgi.getActiveFileDescriptors();
	for (std::vector<ActiveFileDescriptor>::const_iterator it = cgi_fds.begin(); it != cgi_fds.end(); it ++)
		fds.push_back(ActiveFileDescriptor(it->fd, it->mode));

	updateActiveFileDescriptors(fds);
}