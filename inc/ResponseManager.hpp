#pragma once

#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "HTTPRequest.hpp"
#include "HTTPError.hpp"
#include "SysBufferFactory.hpp"
#include "Server.hpp"
#include "HTTPResponseBuffer.hpp"
#include "File.hpp"
#include "ActiveFileDescriptor.hpp"
#include <sys/epoll.h>

class ResponseManager
{
public:
    enum RM_status {
        WAITING_REQUEST, READING_FILE, WRITING_RESPONSE, ERROR, DONE
    };

    ResponseManager(HTTPRequest &, HTTPError &, SysBufferFactory::sys_buffer_type type, int fd);
    ~ResponseManager();

    void set_virtual_server(Server const * config);
    void set_location(Location const * location);
    void generate_response();
    void prepare_file_reading();
    // void generate_response(CGIResponse & response);
    void process();
    bool response_done();
    void new_response();
    void put_headers();
    ActiveFileDescriptor get_active_file_descriptor();

private:
    static const size_t _WRITE_BUFFER_SIZE = 2000;
    static const size_t _READ_FILE_BUFFER_SIZE = 2000;

    HTTPRequest & _request;
    HTTPError & _error;    
    Server const * _config;
    Location const * _location;
    SysBuffer * _sys_buffer;
    HTTPResponseBuffer _buffer;
    RM_status _status;
    File _file;

    void generate_status_response();
    void generate_get_response();
    void read_file();
    void write_response();
    void read_directory();
};