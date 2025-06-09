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
    bool get_crlf_line(std::string & dst);
    bool get_chunk(ssize_t size, std::string & dst);

    ssize_t previous_read_size() const;

private:
    HTTPRequestBuffer(HTTPRequestBuffer &);
    HTTPRequestBuffer & operator=(HTTPRequestBuffer &);
    ssize_t _offset;
    ssize_t _last_read_size;
};