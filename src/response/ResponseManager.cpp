#include "ResponseManager.hpp"

ResponseManager::ResponseManager(HTTPRequest & request, HTTPError & error, SysBufferFactory::sys_buffer_type type, int fd)
:_request(request), _error(error),_status(WAITING_REQUEST){_sys_buffer = SysBufferFactory::get_buffer(type, fd);}

ResponseManager::~ResponseManager(){delete _sys_buffer;}

void ResponseManager::set_virtual_server(Server const * config){_config = config;}

void ResponseManager::set_location(Location const * location){_location = location;}

ActiveFileDescriptor ResponseManager::get_active_file_descriptor()
{
    switch (_status)
    {
        case WAITING_REQUEST:
        case ERROR:
        case DONE:
            CODE_ERR("Trying to get active file descriptor from an invalid status");
        case READING_FILE: return ActiveFileDescriptor(_file.fd, EPOLLIN);
        case WRITING_RESPONSE: return ActiveFileDescriptor(_sys_buffer->_fd, EPOLLOUT);
        default: CODE_ERR("Trying to get active file descriptor from an invalid status");
    }
}

/*
    Called once request is dome
*/
void ResponseManager::generate_response()
{
    switch (::status::status_type(_error.status()))
    {
        case StatusType::STYPE_IMMEDIATE_RESPONSE: generate_status_response(); break;
        case StatusType::STYPE_GENERATE_RESPONSE:
            switch (_request.method)
            {
                case GET: generate_get_response(); break ;
                default: CODE_ERR("Not implemented");
            }
    }
}

void ResponseManager::generate_get_response()
{
    // Find real path (Not for now ...)
    std::string final_path = _request.get_path();

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
        case File::REGULAR: prepare_file_reading(); break;
        case File::DIRECTORY: read_directory(); break;
    }
}

void ResponseManager::prepare_file_reading()
{
    std::string final_path = _request.get_path();
    _buffer.put_header("Server", "Webserv");
    _buffer.put_header_time("Date", time(NULL));
    _buffer.put_header_time("Last-Modified", _file.last_modified());
    _buffer.put_header_number("Content-Length", _file.size());
    _buffer.put_header("Content-Type", MediaType::filename_to_type(final_path));
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
        _status = WRITING_RESPONSE;
    }
}

void ResponseManager::read_directory()
{
    _buffer.put_header("Server", "Webserv");
    _buffer.put_header_time("Date", time(NULL));
    _buffer.put_header_time("Last-Modified", _file.last_modified());
    _buffer.put_header("Content-Type", "text/html");

    if (!_location->hasDirectoryListing())
        return _error.set("Directory listing is forbidden", FORBIDDEN);

    // Find real path (Not for now ...) (This is logical path)
    std::string final_path = _request.get_path();

    _buffer.put_body("<!DOCTYPE html><html><body>");
    while (struct dirent * dir = _file.dir_next())
        _buffer.put_body("<a href=\"" + final_path + "/" + dir->d_name + "\">" + dir->d_name + "</a></hr>");
    _buffer.put_body("</html></body>");

    _status = WRITING_RESPONSE;
}

void ResponseManager::generate_status_response()
{
    _buffer.put_protocol("HTTP/1.1");
    _buffer.put_status(_error);
    _buffer.put_header("Server", "Webserv");
    _buffer.put_header_time("Date", time(NULL));
    // Put headers
    _status = WRITING_RESPONSE;
}

void ResponseManager::process()
{
    switch (_status)
    {
        case WAITING_REQUEST: generate_response(); break;
        case READING_FILE: read_file(); break; 
        case WRITING_RESPONSE: write_response(); break;
        case DONE: throw std::runtime_error("Code error: ResponseManager is DONE but tried to process");
        default: throw std::runtime_error("Code error: Unknown ResponseManager status");
    }
}

void ResponseManager::write_response()
{
    ssize_t written_bytes = _sys_buffer->write(_buffer.get_start(), _WRITE_BUFFER_SIZE);
    if (written_bytes > 0)
    {
        _buffer.consume_bytes(written_bytes);
    }
}

bool ResponseManager::response_done(){return _buffer.size() == 0;}
