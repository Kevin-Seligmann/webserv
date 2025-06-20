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
    static const size_t _READ_BUFFER_SIZE = 2000;

    SysBuffer * _sys_buffer;
    HTTPRequest _request;
    HTTPError _error;
    RequestValidator _validator;
    ElementParser _element_parser;
    RequestParser _request_parser;
    uint8_t _read_buffer[READ_BUFFER_SIZE];
};

RequestManager::RequestManager(HTTPRequest & request, SysBufferFactory::sys_buffer_type type, int fd)
:_validator(_request),
_element_parser(_error),
_request_parser(_request, _error, _element_parser, _validator),
_sys_buffer(SysBufferFactory::get_buffer(type, fd)){}

RequestManager::~RequestManager(){delete _sys_buffer;};

void RequestManager::process()
{
    bool parse = true;

    ssize_t read_size = _sys_buffer->read(_read_buffer, _READ_BUFFER_SIZE);
    if (read_size <= 0)
        return ;
    _request_parser.append(_read_buffer, read_size);
    
    while (_error.status() == OK && parse)
    {
        switch (_request_parser.get_parsing_status())
        {
            case RequestParser::PRS_FIRST_LINE:
                parse = _request_parser.test_first_line(); break;
            case RequestParser::PRS_METHOD:
                _request_parser.parse_method(); break;
            case RequestParser::PRS_URI:
                _request_parser.parse_uri(); break;
            case RequestParser::PRS_PROTOCOL:
                _request_parser.parse_protocol(); break;
            case RequestParser::PRS_HEADER_LINE:
                parse = _request_parser.test_header_line();
                if (parse)
                    _request_parser.parse_header_line();
                break ;
            case RequestParser::PRS_BODY:
                parse = _request_parser.test_body();
                if (parse)
                    _request_parser.parse_body();
                break ;
            case RequestParser::PRS_CHUNKED_SIZE:
                parse = _request_parser.test_chunk_size();
                if (parse)
                    _request_parser.parse_chunked_size();
                break ;
            case RequestParser::PRS_CHUNKED_BODY:
                parse = _request_parser.test_chunk_body();
                if (parse)
                    _request_parser.parse_chunked_body();
                break ;            
            case RequestParser::PRS_TRAILER_LINE:
                parse = _request_parser.test_trailer_line();
                if (parse)
                    _request_parser.parse_trailer_line();
                break ;
            case RequestParser::PRS_DONE: parse = false; break ;  
            default: throw new std::runtime_error("Code error reading Request Parser status");
        }
    }
}