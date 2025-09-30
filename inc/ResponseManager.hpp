#pragma once

#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <poll.h>
#include "HTTPRequest.hpp"
#include "HTTPError.hpp"
#include "SysBufferFactory.hpp"
#include "ServerConfig.hpp"
#include "HTTPResponseBuffer.hpp"
#include "File.hpp"
#include "ActiveFileDescriptor.hpp"
#include "CGI.hpp"

class ResponseManager
{
public:
    enum RM_status {
        WAITING_REQUEST, READING_FILE, WRITING_FILE, WRITING_RESPONSE, ERROR, IDLE
    };

    enum RM_error_action {
        GENERATING_DEFAULT_ERROR_PAGE, GENERATING_LOCATION_ERROR_PAGE
    };

    ResponseManager(CGI &, HTTPRequest &, HTTPError &, SysBufferFactory::sys_buffer_type type, int fd);
    ~ResponseManager();

    void set_virtual_server(ServerConfig const * config);
    void set_location(Location const * location);
    // void generate_response(CGIResponse & response);

    void generate_response(RM_error_action action, bool is_cgi, bool from_autoindex = false);
    void process();
    bool response_done();
    void new_response();
    void set_error_action(RM_error_action action);
    bool is_error();
    const ServerConfig* getServerForResponse() { return _server; }

    ActiveFileDescriptor get_active_file_descriptor();

private:
    static const size_t _WRITE_BUFFER_SIZE = 200000;
    static const size_t _READ_FILE_BUFFER_SIZE = 200000;
    static const size_t _WRITE_FILE_BUFFER_SIZE = 200000;

    HTTPRequest & _request;
    HTTPError & _error;    
    ServerConfig const * _server;
    Location const * _location;
    SysBuffer * _sys_buffer;
    HTTPResponseBuffer _buffer;
    RM_status _status;
    File _file;
    std::string::iterator _wr_file_it;
    RM_error_action _error_action;
    std::string _redirecting_location;
    CGI & _cgi;

    void generate_default_status_response();
    void generate_get_response(bool from_autoindex = false);
    void generate_post_response();
    void generate_delete_response();
    void generate_cgi_response();
    void read_file();
    void write_file();
    void write_response();
    void read_directory();
    void prepare_file_reading();
    void prepare_file_writing();
    bool validate_method();
    void set_error(const std::string & description, Status status);
    void generate_file_status_response();
    // std::string generate_default_error_html();

    std::string const get_host_path();
    std::vector<HTTPMethod> get_allowed_methods();
    bool allow_upload();
    bool is_autoindex();
    
    // How does upload_dir work
    // Return ?
};