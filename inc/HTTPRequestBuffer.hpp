#pragma once

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include "ReadNetBuffer.hpp"

class HTTPRequestBuffer : public ReadNetBuffer
{
public:
    HTTPRequestBuffer();
    ~HTTPRequestBuffer();

    void new_request();
    bool get_crlf_line(uint8_t * & _begin, uint8_t * & _end);
    bool get_chunk(ssize_t size, uint8_t * & _begin, uint8_t * & _end);

    bool get_crlf_line(std::string::iterator & _begin, std::string::iterator &  _end);
    bool get_chunk(ssize_t size, std::string::iterator &  _begin, std::string::iterator &  _end);
    bool get_crlf_chunk(ssize_t chunk_size, std::string::iterator & _begin, std::string::iterator & _end);

    ssize_t previous_read_size() const;

private:
    HTTPRequestBuffer(HTTPRequestBuffer &);
    HTTPRequestBuffer & operator=(HTTPRequestBuffer &);
    ssize_t _offset;
    ssize_t _last_read_size;
};