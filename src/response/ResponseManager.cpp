#include "ResponseManager.hpp"

/* TO_DELETE ya no es necearia la location fake */
Location * lc = new Location();

/*
    Remember status with content, should put C.L 0 or send error page.
*/

ResponseManager::ResponseManager(HTTPRequest & request, HTTPError & error, SysBufferFactory::sys_buffer_type type, int fd)
:_request(request), _error(error),_status(WAITING_REQUEST)
{
    _sys_buffer = SysBufferFactory::get_buffer(type, fd);

    // TO_DELETE
    // TESTING LOCATION CONFIG.
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

void ResponseManager::set_error_action(RM_error_action action){_error_action = action;}

void ResponseManager::set_location(Location const * location)
{
    _location = location;
}

ActiveFileDescriptor ResponseManager::get_active_file_descriptor()
{
    switch (_status)
    {
        case WAITING_REQUEST:
        case ERROR:
        case IDLE:
            CODE_ERR("Trying to get active file descriptor from an invalid status: " + wss::i_to_dec(_status));
        case READING_FILE: return ActiveFileDescriptor(_file.fd, POLLIN);
        case WRITING_FILE: return ActiveFileDescriptor(_file.fd, POLLOUT);
        case WRITING_RESPONSE: return ActiveFileDescriptor(_sys_buffer->_fd, POLLOUT | POLLRDHUP);
        default: CODE_ERR("Trying to get active file descriptor from an invalid status: " + wss::i_to_dec(_status));
    }
}

/*
    Called once request is done
*/
void ResponseManager::generate_response(RM_error_action action)
{
    Logger::getInstance() << "Generating response for client " + wss::i_to_dec((ssize_t) _sys_buffer->_fd) << std::endl;

    _error_action = action;
    switch (::status::status_type(_error.status()))
    {
        case STYPE_BODY_ERROR_RESPONSE:
            if (action == GENERATING_DEFAULT_ERROR_PAGE)
                generate_default_status_response();
            else
                generate_file_status_response();
            break ;
        case STYPE_EMPTY_ERROR_RESPONSE: generate_default_status_response(); break ;
        case STYPE_REGULAR_RESPONSE:
            if (!validate_method())
                return ;
            switch (_request.method)
            {
                case HEAD:
                case GET: generate_get_response(); break ;
                case POST: generate_post_response(); break ;
                case DELETE: generate_delete_response(); break ;
                default: CODE_ERR("Not implemented");
            }
            break ;
        default: CODE_ERR("Not implemented");
    }
}

bool ResponseManager::validate_method()
{
    std::vector<HTTPMethod> methods = get_allowed_methods();
    for (std::vector<HTTPMethod>::iterator it = methods.begin(); it != methods.end(); it ++)
        if (*it == _request.method)
            return true;
    set_error("Method not allowed", METHOD_NOT_ALLOWED);
    return false;
}

void ResponseManager::new_response()
{
    _buffer.clear();
    _status = WAITING_REQUEST;
}

void ResponseManager::generate_get_response()
{
    std::string final_path = get_host_path();

    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ": Generating GET/HEAD response. File: " + final_path + " . Status: " + _error.to_string() << std::endl;

    _file.open(final_path, O_RDONLY);

    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ": Status at opening file: " + wss::ui_to_dec( _file.get_status()) + " Type. " + wss::ui_to_dec( _file.filetype) << std::endl;;

    switch (_file.get_status())
    {
        case File::OK: break;
        case File::NOTFOUND: set_error("File not found", NOT_FOUND); return ;
        case File::NOPERM:  set_error("No access to this file", FORBIDDEN); return ;
        case File::BADFILENAME: set_error("File's basename is invalid", BAD_REQUEST); return ;
        case File::RAREFILE: set_error("File type is not operational", FORBIDDEN); return ;
        case File::ERROR: set_error("Error reading file", INTERNAL_SERVER_ERROR); return ;
    }
    switch (_file.filetype)
    {
        case File::REGULAR: prepare_file_reading(); break;
        case File::DIRECTORY: read_directory(); break;
        case File::NONE: set_error("Rare file type", FORBIDDEN); return ;
    }
}

void ResponseManager::generate_file_status_response()
{
    std::string final_path = get_host_path();

    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ": Generating status response. File: " + final_path + " . Status: " + _error.to_string() << std::endl;

    _file.open(final_path, O_RDONLY);

    // We don't want to change the error type. Just report that there's an error and let the manager handle it.
    if (_file.get_status() != File::OK || _file.filetype != File::REGULAR)
    {
        set_error(_error.msg(), _error.status());
        return ;
    }
    prepare_file_reading();
}

void ResponseManager::generate_post_response()
{
    std::string final_path = get_host_path();
    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) j+ ": Procesing POST. File: " + final_path << std::endl;

    _file.open(final_path, O_WRONLY | O_CREAT | O_APPEND, 0777);
    
    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ": Status at opening file: " + wss::ui_to_dec( _file.get_status()) + " Type. " + wss::ui_to_dec( _file.filetype) << std::endl;;

    switch (_file.get_status())
    {
        case File::OK: break;
        case File::NOTFOUND: set_error("File not found", NOT_FOUND); return ;
        case File::NOPERM:  set_error("No access to this file", FORBIDDEN); return;
        case File::BADFILENAME: set_error("File's basename is invalid", BAD_REQUEST); return ;
        case File::RAREFILE: set_error("File type is not operational", FORBIDDEN); return;
        case File::ERROR: set_error("Error reading file", INTERNAL_SERVER_ERROR); return;
    }
    switch (_file.filetype)
    {
        case File::REGULAR: break;
        case File::DIRECTORY: set_error("Can't delete a directory", FORBIDDEN); return ;
        case File::NONE: set_error("Rare file type", FORBIDDEN); return ;
    }

    if (_file.creation_status == File::NEW)
        _error.set("File created", CREATED);
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
        case File::NOTFOUND: set_error("File not found", NOT_FOUND); return ;
        case File::NOPERM:  set_error("No access to this file", FORBIDDEN); return ;
        case File::BADFILENAME: set_error("File's basename is invalid", BAD_REQUEST); return ;
        case File::RAREFILE: set_error("File type is not operational", FORBIDDEN); return ;
        case File::ERROR: set_error("Error reading file", INTERNAL_SERVER_ERROR); return ;
    }
    switch (_file.filetype)
    {
        case File::REGULAR: break;
        case File::DIRECTORY: set_error("Can't delete a directory", FORBIDDEN); return ;
        case File::NONE: set_error("Rare file type", FORBIDDEN); return ;
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
    if (_error.status() == MOVED_PERMANENTLY)
        _buffer.put_header("Location", _redirecting_location);
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
        set_error("Error reading file", INTERNAL_SERVER_ERROR);
        return ;
    }
    if (bytes_read == 0)
    {
        Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ". File read. Full planned response: \n" + std::string(_buffer.itbegin(), _buffer.itend()) << std::endl;
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
        set_error("Error writing file", INTERNAL_SERVER_ERROR);
        return ;
    }

    // Advance the content pointer
    _wr_file_it += bytes_writen;
    if (_wr_file_it == _request.body.content.end())
    {
        _buffer.put_protocol("HTTP/1.1");
        _buffer.put_status(_error);
        _buffer.put_header("Server", "Webserv");
        _buffer.put_header_time("Date", time(NULL));
        _buffer.put_header_number("Content-Length", 0);
        if(_error.status() == CREATED)
        {
            _buffer.put_header("Location", get_host_path());
        }
        _buffer.put_new_line();
        Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ". File writen. Full planned response: \n" + std::string(_buffer.itbegin(), _buffer.itend()) << std::endl;
        _status = WRITING_RESPONSE;
    }
}


void ResponseManager::read_directory()
{
    std::string final_path = get_host_path();

    if (_request.get_path().at(_request.get_path().size() - 1) != '/')
    {
        set_error("The user requested a directory that is a file but doesn't end with a backslash", MOVED_PERMANENTLY);
        _redirecting_location = _request.get_path() + "/";
        return ;
    }            

    _buffer.put_protocol("HTTP/1.1");
    _buffer.put_status(_error);
    _buffer.put_header("Server", "Webserv");
    _buffer.put_header_time("Date", time(NULL));
    _buffer.put_header_time("Last-Modified", _file.last_modified());
    _buffer.put_header("Content-Type", "text/html");

    // Tester asks for NOT_FOUND. Honestly, makes more sense than FORBIDDEN (As: Index not found)
    if (!is_autoindex())
        return set_error("Directory listing is forbidden", NOT_FOUND);

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
    if (_request.method != HEAD)
        _buffer.put_body(dirs);

    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ". Full planned response: \n" + std::string(_buffer.itbegin(), _buffer.itend()) << std::endl;
    _status = WRITING_RESPONSE;
}

void ResponseManager::generate_default_status_response()
{
    Logger::getInstance() << "Generating status for client " + wss::ui_to_dec( _sys_buffer->_fd) << std::endl;

    _buffer.put_protocol("HTTP/1.1");
    _buffer.put_status(_error);
    _buffer.put_header("Server", "Webserv");
    _buffer.put_header_time("Date", time(NULL));

    if (::status::status_type(_error.status()) == STYPE_EMPTY_ERROR_RESPONSE)
    {
        _buffer.put_header_number("Content-Length", 0);
        _buffer.put_new_line();
    }
    else 
    {
        std::string error_page = "Error with status: " + status::status_to_text(_error.status());
        _buffer.put_header_number("Content-Length", error_page.size());
        _buffer.put_header("Content-Type", MediaType::filename_to_type(".txt"));
        if (_error.status() == MOVED_PERMANENTLY)
            _buffer.put_header("Location", _redirecting_location);
        _buffer.put_new_line();
        if (_request.method != HEAD)
            _buffer.put_body(error_page);
    }


    Logger::getInstance() << wss::ui_to_dec( _sys_buffer->_fd) + ". Full planned response: \n" + std::string(_buffer.itbegin(), _buffer.itend()) << std::endl;
    _status = WRITING_RESPONSE;
}

void ResponseManager::process()
{
    switch (_status)
    {
        case READING_FILE: read_file(); break; 
        case WRITING_FILE: write_file(); break ;
        case WRITING_RESPONSE: write_response(); break;
        case IDLE: throw std::runtime_error("Code error: ResponseManager is IDLE but tried to process");
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
        _status = IDLE;
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

    std::string path;
    if (_location)
        path = _location->getFilesystemLocation(_request.get_path());
    if (path.empty() && !_server->getRoot().empty())
        path = _server->getRoot() + _request.get_path();
    else if (path.empty())
        CODE_ERR("No root path found for " + _request.get_path() + " Server: " + _server->getRoot());
    return path;
}

std::vector<HTTPMethod> ResponseManager::get_allowed_methods()
{
    std::vector<std::string> methods;
    std::vector<HTTPMethod> real_methods;

    if (_location && !_location->getMethods().empty()) {
        methods = _location->getMethods();
    } 
    else if (!_server->getAllowMethods().empty()) {
        methods = _server->getAllowMethods();
    } 
    else {
        CODE_ERR("No methods found");
    }

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
    return _location && _location->getAllowUpload();
    // return true;
}

// fixed at ConfigInheritance.cpp  applyAutoindex
bool ResponseManager::is_autoindex()
{
    if (_location && _location->getAutoindex() != AINDX_DEF_OFF) {
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

void ResponseManager::set_error(const std::string & description, Status status)
{
    Logger::getInstance() << "Error responding: " + description + ". " + status::status_to_text(status) << std::endl;
    _error.set(description, status);
    // if (_error_action == GENERATING_LOCATION_ERROR_PAGE)
        _status = ERROR;
}

bool ResponseManager::is_error()
{
    return _status == ERROR;
}