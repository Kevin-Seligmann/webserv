#pragma once

#include <string>
#include <iostream>
#include <stdint.h>
#include <stdexcept>
#include "HTTPRequest.hpp"
#include "HTTPRequestBuffer.hpp"

enum parsing_status {FIRST_LINE, HEADERS, BODY, DONE};

class RequestParser 
{
public:
    RequestParser(HTTPRequest & request);
    void append(uint8_t *str, ssize_t size);
    void process();
    bool done() const;
    void new_request();
    void dump_remainer() const;

private:
    static const std::string ILLEGAL_CHAR;

    HTTPRequest & _request;
    HTTPRequestBuffer _buffer;
    parsing_status _status;

    void sanitize(std::string & str) const;
    bool illegal(char c) const;
};