#pragma once

#include "HTTPRequest.hpp"
#include "HTTPError.hpp"
#include "SysBufferFactory.hpp"
#include "Servers.hpp"

class ResponseManager
{
public:
    ResponseManager(HTTPRequest, HTTPError, SysBufferFactory::sys_buffer_type type, int fd);

    void set_virtual_server(Servers const * config);
    void process();
    bool response_done();
    void new_response();

private:
    static const size_t _WRITE_BUFFER_SIZE = 2000;

    Servers const * _config;
    SysBuffer * _sys_buffer;
    HTTPRequest & _request;
    HTTPError _error;
    uint8_t _write_buffer[_WRITE_BUFFER_SIZE];

};