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
#include "StreamRequest.hpp"

class RequestManager 
{
public:
    RequestManager(HTTPRequest & request, HTTPError & error, SysBufferFactory::sys_buffer_type type, int fd, StreamRequest & stream_request);
    ~RequestManager();

    void process();
    bool request_done() const;
    bool has_error() const;
    bool close() const;
    void new_request();
    HTTPError & gerError();

private:
    enum processing_type {STANDARD, STREAM};

    static const size_t _READ_BUFFER_SIZE = 2000;

    HTTPRequest & _request;
    HTTPError & _error;
    RequestValidator _validator;
    ElementParser _element_parser;
    RequestParser _request_parser;
    SysBuffer * _sys_buffer;
    uint8_t _read_buffer[_READ_BUFFER_SIZE];
    StreamRequest & _stream_request;
    enum processing_type _processing_type;

    size_t chunk_size;
    size_t chunk_read;
    size_t read_buffer_space;

    void process_stream();
    void set_streaming(bool has_read);
    void unset_streaming(bool has_read);
};