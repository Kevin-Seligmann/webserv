#pragma once

#include <string>
#include <iostream>
#include <stdint.h>
#include <stdexcept>
#include <sstream>
#include "HTTPRequest.hpp"
#include "HTTPRequestBuffer.hpp"
#include "RequestValidator.hpp"
#include "StringUtil.hpp"
#include "ParsingUtil.hpp"

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
    //void set_validator();

private:
    static const std::string ILLEGAL_CHAR;

    HTTPRequest & _request;
    HTTPRequestBuffer _buffer;
    parsing_status _status;
    RequestValidator _validator;

    bool _processing;
    std::string _line;
    int _empty_skip_count;

    void sanitize(std::string & str) const;
    bool illegal(char c) const;
    void parse_first_line();
    void parse_header_line();
    void parse_body();

    void get_first_line_words(std::string const & src, std::string & method, std::string & uri, std::string & protocol);
};