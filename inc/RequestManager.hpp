#pragma once

#include <stdexcept>
#include "HTTPRequest.hpp"
#include "RequestValidator.hpp"
#include "RequestManager.hpp"
#include "ElementParser.hpp"
#include "RequestParser.hpp"
#include "SysBuffer.hpp"
#include "SysFileBuffer.hpp"
#include "SysNetBuffer.hpp"

class RequestManager 
{
public:
    enum system_buffer_type {SBT_NET, SBT_FILE};

    RequestManager(system_buffer_type type, int fd);
    ~RequestManager();

    void process();

private:
    SysBuffer * _sys_buffer;
    HTTPRequest _request;
    ErrorContainer _error_container;
    RequestValidator _validator;
    ElementParser _element_parser;
    RequestParser _request_parser;
};

RequestManager::RequestManager(system_buffer_type type, int fd)
:_validator(_request, _error_container),
_element_parser(_error_container),
_request_parser(_request, _error_container, _element_parser, _validator)
{
    switch (type)
    {
        case SBT_NET: _sys_buffer = new SysFileBuffer(fd); break;
        case SBT_FILE: _sys_buffer = new SysNetBuffer(fd); break;
        default: throw std::runtime_error("Code error, invalid enum");
    }
}

RequestManager::~RequestManager(){delete _sys_buffer;};

void process()
{
    
}