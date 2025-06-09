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
#include "ErrorContainer.hpp"
#include "ElementParser.hpp"

// struct headerType {
//     char *name;
//     bool singleton;
//     void (RequestParser::*parser_f)(std::string const & value);
// };

enum parsing_status {FIRST_LINE, HEADERS, BODY, DONE};

class RequestParser 
{
public:
    static const size_t URI_MAX_LENGTH;
    static const size_t FIRST_LINE_MAX_LENGTH;
    static const size_t HEADER_LINE_MAX_LENGTH;
    static const size_t MAX_CONTENT_LENGTH;
    static const size_t MAX_HEADER_FIELDS;
    static const size_t MAX_TRAILER_FIELDS;

    RequestParser(HTTPRequest & request, ErrorContainer & error_container, ElementParser & _element_parser, RequestValidator & validator);
    void append(uint8_t *str, ssize_t size);
    void process();
    bool done() const;
    void new_request();
    void dump_remainder() const;

private:
    struct wsHeaders {
        std::string name;
        void (RequestParser::*parser_f)(std::string const & value);
    };

    static const wsHeaders headers[];

    HTTPRequestBuffer _buffer;

    HTTPRequest & _request;
    ErrorContainer & _error_container;
    ElementParser &  _element_parser;
    RequestValidator & _validator;

    parsing_status _status;
    int _empty_skip_count;
    bool _processing;
    size_t _header_field_count;

    std::string::const_iterator _token_start, _token_end;
    std::string _line;

    void percentage_decode(std::string & str);

    void parse_first_line();
    void parse_header_line();
    void parse_body();

    void normalize_path(std::string & str);
    void replace_percentage(std::string::iterator & it, std::string & str);

    void get_method();
    void get_protocol();
    void parse_uri();
    void get_path();
    void get_hier_part(); 
    void get_query();
    void get_fragment();
    void get_schema();
    bool has_authority() const;

    void process_headers();
    void parse_host_field(std::string const & value);
    void parse_content_length_field(std::string const & value);
};