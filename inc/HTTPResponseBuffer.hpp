#pragma once

#include <cstdlib>
#include <sstream>
#include <ctime>
#include <unistd.h>
#include "StringUtil.hpp"
#include "HTTPMethod.hpp"
#include "ReadNetBuffer.hpp"
#include "HTTPError.hpp"

class HTTPResponseBuffer : public ReadNetBuffer
{
public:
    void put_protocol(std::string const & protocol);
    void put_status(HTTPError const & err);
    void put_header(std::string const & name, std::string const & value);
    void put_header_time(std::string const & name, time_t time);
    void put_header_number(std::string const & name, size_t n);
    void put_body(std::string const & body);
    void put_body_chunk(std::string const & body);
    void put_new_line();
    
    ssize_t write_from_fd(int fd, size_t n);
};