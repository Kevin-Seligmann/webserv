#pragma once

#include <stdexcept>
#include "HTTPRequest.hpp"
#include "RequestValidator.hpp"
#include "RequestManager.hpp"
#include "ElementParser.hpp"
#include "RequestParser.hpp"
#include "SysBufferFactory.hpp"
#include "SysBuffer.hpp"
#include "SysFileBuffer.hpp"
#include "SysNetBuffer.hpp"

class RequestManager 
{
public:
    RequestManager(HTTPRequest & request, SysBufferFactory::sys_buffer_type type, int fd);
    ~RequestManager();

    void process();

private:
    SysBuffer * _sys_buffer;
    HTTPRequest _request;
    HTTPError _error;
    RequestValidator _validator;
    ElementParser _element_parser;
    RequestParser _request_parser;
};

RequestManager::RequestManager(HTTPRequest & request, SysBufferFactory::sys_buffer_type type, int fd)
:_validator(_request, _error_container),
_element_parser(_error_container),
_request_parser(_request, _error_container, _element_parser, _validator),
_sys_buffer(SysBufferFactory::get_buffer(type, fd)){}

RequestManager::~RequestManager(){delete _sys_buffer;};

void RequestManager::process()
{
    
}