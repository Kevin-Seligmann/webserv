#pragma once

#include <string>
#include <iostream>
#include <stdint.h>
#include <stdexcept>
#include <sstream>
#include <stdlib.h>
#include "Logger.hpp"
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
    bool error() const;
    //void set_validator();

private:
    HTTPRequest & _request;
    HTTPRequestBuffer _buffer;
    parsing_status _status;
    RequestValidator _validator;
    std::string::const_iterator _token_start, _token_end;

    bool _processing;
    std::string _line;
    int _empty_skip_count;

    void percentage_decode(std::string & str);

    void parse_first_line();
    void parse_header_line();
    void parse_body();

    void normalize_path(std::string & str);
    void replace_percentage(std::string::iterator & it, std::string & str);

    void get_method();
    void get_protocol();
    void parse_uri();
    void get_absolute_path();
    void get_hier_part(); 
    void get_path_absolute();
    void get_path_rootless_or_empty();
    void get_query();
    void get_fragment();
    void get_schema();
    bool has_authority();

};