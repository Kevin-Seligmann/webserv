#pragma once

#include <cstdlib>
#include "StringUtil.hpp"
#include "HTTPMethod.hpp"
#include "ReadNetBuffer.hpp"
#include "HTTPError.hpp"

class HTTPResponseBuffer : public ReadNetBuffer
{
    void put_protocol(std::string const & protocol);
    void put_status(HTTPError const & err);
    void put_header(std::string const & name, std::string const & value);
    void put_body(std::string const & body);
    void put_body_chunk(std::string const & body);
    void put_new_line();
};