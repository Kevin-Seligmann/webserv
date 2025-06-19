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
#include "HTTPError.hpp"
#include "ElementParser.hpp"

// struct headerType {
//     char *name;
//     bool singleton;
//     void (RequestParser::*parser_f)(std::string const & value);
// };

enum parsing_status {FIRST_LINE, HEADERS, BODY, CHUNKED_SIZE, CHUNKED_BODY, TRAILERS, DONE};

class RequestParser 
{
public:
    static const size_t URI_MAX_LENGTH;
    static const size_t FIRST_LINE_MAX_LENGTH;
    static const size_t HEADER_LINE_MAX_LENGTH;
    static const size_t MAX_CONTENT_LENGTH;
    static const size_t MAX_HEADER_FIELDS;
    static const size_t MAX_TRAILER_FIELDS;
    static const size_t MAX_CHUNK_SIZE;
    static const size_t CHUNKED_SIZE_LINE_MAX_LENGTH;

    RequestParser(HTTPRequest & request, HTTPError & _error, ElementParser & _element_parser, RequestValidator & validator);
    void append(uint8_t const * str, ssize_t size);
    void process();
    bool done() const;
    void new_request();
    void dump_remainder() const;
    std::string const get_remainder() const;

private:
    struct wsHeaders {
        std::string name;
        void (RequestParser::*parser_f)(std::string const & value);
    };

    static const wsHeaders headers[];

    HTTPRequestBuffer _buffer;

    HTTPRequest & _request;
    HTTPError & _error;
    ElementParser &  _element_parser;
    RequestValidator & _validator;

    parsing_status _status;
    int _empty_skip_count;
    bool _processing;
    size_t _header_field_count;
    size_t _trailer_field_count;
    size_t _chunk_length;

    std::string::const_iterator _token_start, _token_end;
    std::string _line;

    void percentage_decode(std::string & str);

    void parse_first_line();
    void parse_header_line();
    void parse_body();
    void parse_trailer_line();
    void parse_chunked_size();
    void parse_chunked_body();

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
    void parse_transfer_encoding_field(std::string const & value);
};