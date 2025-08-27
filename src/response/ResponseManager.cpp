#include "ResponseManager.hpp"

Location * lc = new Location();

/*
    Remember status with content, should put C.L 0 or send error page.
*/

ResponseManager::ResponseManager(HTTPRequest & request, HTTPError & error, SysBufferFactory::sys_buffer_type type, int fd)
:_request(request), _error(error),_status(WAITING_REQUEST)
{
    _sys_buffer = SysBufferFactory::get_buffer(type, fd);

    // // TESTING LOCATION CONFIG.
    // lc->setPath("/def/");
    // lc->setRoot("/home/kevin/42/webserv/test/net-request-tests");

    // std::vector<std::string> m;
    // m.push_back("GET");
    // m.push_back("POST");
    // m.push_back("DELETE");
    // lc->setMethods(m);

    // this->_location = lc;
}

ResponseManager::~ResponseManager(){delete _sys_buffer;}

void ResponseManager::set_virtual_server(ServerConfig const * config){_server = config;}

void ResponseManager::set_location(Location const * location)
{
    // _location = location;
}

ActiveFileDescriptor ResponseManager::get_active_file_descriptor()
{
    switch (_status)
    {
        case WAITING_REQUEST:
        case ERROR:
        case DONE:
            CODE_ERR("Trying to get active file descriptor from an invalid status");
        case READING_FILE: return ActiveFileDescriptor(_file.fd, EPOLLIN);
        case WRITING_FILE: return ActiveFileDescriptor(_file.fd, EPOLLOUT);
        case WRITING_RESPONSE: return ActiveFileDescriptor(_sys_buffer->_fd, EPOLLOUT | EPOLLRDHUP);
        default: CODE_ERR("Trying to get active file descriptor from an invalid status");
    }
}

/*
    Called once request is done
*/
void ResponseManager::generate_response()
{
    Logger::getInstance() << "Generating response for client " + wss::i_to_dec((ssize_t) _sys_buffer->_fd) << std::endl;

    switch (::status::status_type(_error.status()))
    {
        case STYPE_IMMEDIATE_RESPONSE: generate_status_response(); break;
        case STYPE_GENERATE_RESPONSE:
            if (!validate_method())
                return generate_status_response();
            switch (_request.method)
            {
                case GET: generate_get_response(); break ;
                case POST: generate_post_response(); break ;
                case DELETE: generate_delete_response(); break ;
                default: CODE_ERR("Not implemented");
            }
    }
}

bool ResponseManager::validate_method()
{
    std::vector<HTTPMethod> methods = get_allowed_methods();
    for (std::vector<HTTPMethod>::iterator it = methods.begin(); it != methods.end(); it ++)
        if (*it == _request.method)
            return true;
    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ": method not allowed " + method::method_to_str(_request.method) << std::endl;
    _error.set("Method not allowed", METHOD_NOT_ALLOWED);
    return false;
}

void ResponseManager::new_response()
{
    _buffer.clear();
    _server = NULL;
    _location = NULL;
    _status = WAITING_REQUEST;
}

void ResponseManager::generate_get_response()
{
    std::string final_path = get_host_path();

    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ": Generating GET response. File: " + final_path << std::endl;

    _file.open(final_path, O_RDONLY);

    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ": Status at opepning file: " + wss::ui_to_dec( _file.get_status()) + " Type. " + wss::ui_to_dec( _file.filetype) << std::endl;;

    switch (_file.get_status())
    {
        case File::OK: break;
        case File::NOTFOUND: _error.set("File not found", NOT_FOUND); return generate_status_response();
        case File::NOPERM:  _error.set("No access to this file", FORBIDDEN); return generate_status_response();
        case File::BADFILENAME: _error.set("File's basename is invalid", BAD_REQUEST); return generate_status_response();
        case File::RAREFILE: _error.set("File type is not operational", FORBIDDEN); return generate_status_response();
        case File::ERROR: _error.set("Error reading file", INTERNAL_SERVER_ERROR); return generate_status_response();
    }
    switch (_file.filetype)
    {
        case File::REGULAR: prepare_file_reading(); break;
        case File::DIRECTORY: read_directory(); break;
        case File::NONE: _error.set("Rare file type", FORBIDDEN); return generate_status_response();
    }
}

void ResponseManager::generate_post_response()
{
    std::string final_path = get_host_path();
    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ": Procesing POST. File: " + final_path << std::endl;

    _file.open(final_path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ": Status at opepning file: " + wss::ui_to_dec( _file.get_status()) + " Type. " + wss::ui_to_dec( _file.filetype) << std::endl;;

    switch (_file.get_status())
    {
        case File::OK: break;
        case File::NOTFOUND: _error.set("File not found", NOT_FOUND); return generate_status_response();
        case File::NOPERM:  _error.set("No access to this file", FORBIDDEN); return generate_status_response();
        case File::BADFILENAME: _error.set("File's basename is invalid", BAD_REQUEST); return generate_status_response();
        case File::RAREFILE: _error.set("File type is not operational", FORBIDDEN); return generate_status_response();
        case File::ERROR: _error.set("Error reading file", INTERNAL_SERVER_ERROR); return generate_status_response();
    }
    switch (_file.filetype)
    {
        case File::REGULAR: break;
        case File::DIRECTORY: _error.set("Can't overwrite a directory", FORBIDDEN); return generate_status_response();
        case File::NONE: _error.set("Rare file type", FORBIDDEN); return generate_status_response();
    }

    _wr_file_it = _request.body.content.begin();
    _status = WRITING_FILE;
};

void ResponseManager::generate_delete_response()
{
    std::string final_path = get_host_path();
    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ": Procesing DELETE. File: " + final_path << std::endl;
    
    _file.open(final_path, O_RDONLY);

    switch (_file.get_status())
    {
        case File::OK: break;
        case File::NOTFOUND: _error.set("File not found", NOT_FOUND); return generate_status_response();
        case File::NOPERM:  _error.set("No access to this file", FORBIDDEN); return generate_status_response();
        case File::BADFILENAME: _error.set("File's basename is invalid", BAD_REQUEST); return generate_status_response();
        case File::RAREFILE: _error.set("File type is not operational", FORBIDDEN); return generate_status_response();
        case File::ERROR: _error.set("Error reading file", INTERNAL_SERVER_ERROR); return generate_status_response();
    }
    switch (_file.filetype)
    {
        case File::REGULAR: break;
        case File::DIRECTORY: _error.set("Can't delete a directory", FORBIDDEN); return generate_status_response();
        case File::NONE: _error.set("Rare file type", FORBIDDEN); return generate_status_response();
    }

    _file.close();
    std::remove(final_path.c_str());
};


void ResponseManager::prepare_file_reading()
{
    std::string final_path = get_host_path();

    _buffer.put_protocol("HTTP/1.1");
    _buffer.put_status(_error);
    _buffer.put_header("Server", "Webserv");
    _buffer.put_header_time("Date", time(NULL));
    _buffer.put_header_time("Last-Modified", _file.last_modified());
    _buffer.put_header_number("Content-Length", _file.size());
    _buffer.put_header("Content-Type", MediaType::filename_to_type(final_path));
    _buffer.put_new_line();
    _status = READING_FILE;
}

/*
    Each iteration reads from a file and puts the result in the internal buffer.
    When the file's end has been reached, it sets the flag to response.

    If there's a problem reading the file, sets internal error status (File should exists and be ok, cheked before). Wipes the
    current headers, and returns to response loop.
*/
void ResponseManager::read_file()
{
    ssize_t bytes_read = _buffer.write_from_fd(_file.fd, ResponseManager::_READ_FILE_BUFFER_SIZE);
    if (bytes_read < 0)
    {
        _buffer.clear();
        _status = WAITING_REQUEST ;
        _error.set("Error reading file", INTERNAL_SERVER_ERROR);
        return generate_status_response();
    }
    if (bytes_read == 0)
    {
        Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ". File read. Full planned response: " + std::string(_buffer.itbegin(), _buffer.itend()) << std::endl;
        _status = WRITING_RESPONSE;
    }
}

void ResponseManager::write_file()
{
    size_t write_qty = ResponseManager::_WRITE_FILE_BUFFER_SIZE;
    // Just writes from the body's pointer to the end or write limit
    ssize_t bytes_writen = write(_file.fd, &(*_wr_file_it), std::min(write_qty, (size_t) std::distance(_wr_file_it, _request.body.content.end())));

    if (bytes_writen < 0)
    {
        _status = WAITING_REQUEST ;
        _error.set("Error writing file", INTERNAL_SERVER_ERROR);
        return generate_status_response();
    }

    // Advance the content pointer
    _wr_file_it += bytes_writen;
    if (_wr_file_it == _request.body.content.end())
    {
        _buffer.put_protocol("HTTP/1.1");
        _buffer.put_status(_error);
        _buffer.put_header("Server", "Webserv");
        _buffer.put_header_time("Date", time(NULL));
        Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ". File writen. Full planned response: \n" + std::string(_buffer.itbegin(), _buffer.itend()) << std::endl;
        _status = WRITING_RESPONSE;
    }
}


void ResponseManager::read_directory()
{
    _buffer.put_protocol("HTTP/1.1");
    _buffer.put_status(_error);
    _buffer.put_header("Server", "Webserv");
    _buffer.put_header_time("Date", time(NULL));
    _buffer.put_header_time("Last-Modified", _file.last_modified());
    _buffer.put_header("Content-Type", "text/html");

    if (!is_autoindex())
        return _error.set("Directory listing is forbidden", FORBIDDEN);

    std::string final_path = get_host_path();
    std::string dir_prefix = "";
    if (!_request.uri.path.empty() && _request.uri.path[_request.uri.path.size() - 1] != '/')
        dir_prefix = _request.uri.path + "/";
    else
        dir_prefix = _request.uri.path;
    
    std::string dirs;

    dirs += "<!DOCTYPE html><html><body>";
    while (struct dirent * dir = _file.dir_next())
        if (dir->d_type == DT_DIR)
            dirs += "<a href=\"" + dir_prefix + dir->d_name + "\">" + dir->d_name + "</a><hr>";
        else
            dirs += "<a href=\"" + dir_prefix + dir->d_name + "\">" + dir->d_name + "</a><hr>";
    dirs += "</html></body>";

    _buffer.put_header("Content-Length", wss::i_to_dec(dirs.size()));
    _buffer.put_new_line();
    _buffer.put_body(dirs);

    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ". Full planned response: " + std::string(_buffer.itbegin(), _buffer.itend()) << std::endl;
    _status = WRITING_RESPONSE;
}

void ResponseManager::generate_status_response()
{
    Logger::getInstance() << "Generating status for client " + wss::ui_to_dec( _sys_buffer->_fd) << std::endl;

    _buffer.put_protocol("HTTP/1.1");
    _buffer.put_status(_error);
    _buffer.put_header("Server", "Webserv");
    _buffer.put_header_time("Date", time(NULL));

    if (_error.status() == METHOD_NOT_ALLOWED)
    {
        std::vector<HTTPMethod> methods = get_allowed_methods();

        std::string allowed;
        for (std::vector<HTTPMethod>::iterator it = methods.begin(); it != methods.end(); it ++)
        {
            if (it != methods.begin())
                allowed += ", ";
            allowed += method::method_to_str(*it);
        }
        _buffer.put_header("Allow", allowed);
    }


    _buffer.put_new_line();
    // Put headers

    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ". Full planned response: \n" + std::string(_buffer.itbegin(), _buffer.itend()) << std::endl;
    _status = WRITING_RESPONSE;
}

void ResponseManager::process()
{
    switch (_status)
    {
        case WAITING_REQUEST: generate_response(); break;
        case READING_FILE: read_file(); break; 
        case WRITING_FILE: write_file(); break ;
        case WRITING_RESPONSE: write_response(); break;
        case DONE: throw std::runtime_error("Code error: ResponseManager is DONE but tried to process");
        default: throw std::runtime_error("Code error: Unknown ResponseManager status");
    }
}

void ResponseManager::write_response()
{
    size_t max = _WRITE_BUFFER_SIZE;
    size_t write_qty = std::min<size_t>(max, _buffer.size());
    ssize_t written_bytes = _sys_buffer->write(_buffer.get_start(), write_qty);
    if (written_bytes > 0)
    {
        _buffer.consume_bytes(written_bytes);
    }
    else if (written_bytes == 0)
    {
        _buffer.clear();
        _file.close();
        _status = DONE;
    }
    else
    {
        Logger & i = Logger::getInstance();
        i.error("Writing response, something went wrong with the operation. Can't reply with an status to the client. Must close: " + std::string(strerror(errno)));
        CODE_ERR("Writing response, something went wrong with the operation. Can't reply with an status to the client. Must close.");
    }
}

bool ResponseManager::response_done(){return _buffer.size() == 0;}


std::string const ResponseManager::get_host_path()
{
    // DEFAULT. Default should be set in config, not here.
    // Removed null check because they should never be empty.
    // root_path = "/var/www/html";

    if (!_location->getRoot().empty()) {
        return _location->getRoot();
    } else if (!_server->getRoot().empty()) {
        return _server->getRoot();
    } else {
        CODE_ERR("No root path found");
    }

    return _location->getRoot() + _request.uri.path;
}

std::vector<HTTPMethod> ResponseManager::get_allowed_methods()
{
    std::vector<std::string> const & methods = _location->getMethods();
    std::vector<HTTPMethod> real_methods;

    if (!_location->getMethods().empty()) {
        methods == _location->getMethods();
    } else if (!_server->getAllowMethods().empty()) {
        methods == _server->getAllowMethods();
    } else {
        CODE_ERR("No methods found");
    }

    // Tempx2 (Parse to enums just once)
    for (std::vector<std::string>::const_iterator it = methods.begin(); it != methods.end(); it ++)
    {
        HTTPMethod m = method::str_to_method(*it);
        if (m != NOMETHOD)
            real_methods.push_back(m);
    }

    return real_methods;
}

bool ResponseManager::allow_upload()
{
    return _location->getAllowUpload();
    // return true;
}
    
bool ResponseManager::is_autoindex()
{
    if (_location->getAutoindex() != AINDX_DEF_OFF) {
        return _location->getAutoindex() == AINDX_LOC_ON;
    }
    
    if (_server) {
        AutoIndexState serv_state = _server->getAutoindex(NULL);
        if (serv_state == AINDX_SERV_ON) return true;
        if (serv_state == AINDX_SERV_OFF) return false;
    }
    
    return false;  // DEFAULT: FALSE
    // return _location->hasAutoindex();
}