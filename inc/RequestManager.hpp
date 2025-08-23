#pragma once

#include <stdexcept>
#include "HTTPRequest.hpp"
#include "RequestValidator.hpp"
#include "ElementParser.hpp"
#include "RequestParser.hpp"
#include "SysBufferFactory.hpp"
#include "SysBuffer.hpp"
#include "SysFileBuffer.hpp"
#include "SysNetBuffer.hpp"

class RequestManager 
{
public:
    RequestManager(HTTPRequest & request, HTTPError & error, SysBufferFactory::sys_buffer_type type, int fd);
    ~RequestManager();

    void process();
    bool request_done() const;
    bool has_error() const;
    void new_request();
    HTTPError & gerError();

private:
    static const size_t _READ_BUFFER_SIZE = 2000;

    HTTPRequest & _request;
    HTTPError & _error;
    RequestValidator _validator;
    ElementParser _element_parser;
    RequestParser _request_parser;
    SysBuffer * _sys_buffer;
    uint8_t _read_buffer[_READ_BUFFER_SIZE];
};