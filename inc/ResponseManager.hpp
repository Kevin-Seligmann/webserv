#pragma once

#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstdio>
#include "HTTPRequest.hpp"
#include "HTTPError.hpp"
#include "SysBufferFactory.hpp"
#include "HTTPResponseBuffer.hpp"
#include "File.hpp"
#include "ActiveFileDescriptor.hpp"

// FORWARD DECLARATIONS TO AVOID CIRCULAR DEPENDENCIES
class ServerConfig;
class Location;

class ResponseManager
{
public:
    enum RM_status {
        WAITING_REQUEST, READING_FILE, WRITING_FILE, WRITING_RESPONSE, ERROR, DONE
    };

    ResponseManager(HTTPRequest &, HTTPError &, SysBufferFactory::sys_buffer_type type, int fd);
    ~ResponseManager();

    void set_virtual_server(ServerConfig const * config); // Cambiado: Server -> ServerConfig
    void set_location(Location const * location);
    void generate_response();
    // void generate_response(CGIResponse & response);
    void process();
    bool response_done();

    ActiveFileDescriptor get_active_file_descriptor();

private:
    static const size_t _WRITE_BUFFER_SIZE = 2000;
    static const size_t _READ_FILE_BUFFER_SIZE = 2000;
    static const size_t _WRITE_FILE_BUFFER_SIZE = 2000;

    HTTPRequest & _request;
    HTTPError & _error;    
    RM_status _status;
    ServerConfig const * _server; // Cambiado: Server -> ServerConfig
    Location const * _location;
    SysBuffer * _sys_buffer;
    HTTPResponseBuffer _buffer;
    File _file;
    std::string::iterator _wr_file_it;

    void generate_status_response();
    void generate_get_response();
    void generate_post_response();
    void generate_delete_response();
    void read_file();
    void write_file();
    void write_response();
    void read_directory();
    void prepare_file_reading();
    void prepare_file_writing();
    void new_response();
    bool validate_method();

    std::string const get_host_path();
    std::vector<HTTPMethod> get_allowed_methods();
    bool allow_upload();
    bool is_autoindex();
    
    // How does upload_dir work
    // Return ?
};