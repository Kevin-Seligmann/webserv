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

private:
    HTTPRequestBuffer(HTTPRequestBuffer &);
    HTTPRequestBuffer & operator=(HTTPRequestBuffer &);
};