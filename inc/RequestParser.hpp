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


class RequestParser 
{
public:
    enum parsing_status {PRS_FIRST_LINE, PRS_METHOD, PRS_URI, PRS_PROTOCOL, PRS_HEADER_LINE, PRS_BODY, PRS_CHUNKED_SIZE, PRS_CHUNKED_BODY, PRS_TRAILER_LINE, PRS_DONE};

    static const size_t URI_MAX_LENGTH;
    static const size_t FIRST_LINE_MAX_LENGTH;
    static const size_t HEADER_LINE_MAX_LENGTH;
    static const size_t MAX_CONTENT_LENGTH;
    static const size_t MAX_HEADER_FIELDS;
    static const size_t MAX_TRAILER_FIELDS;
    static const size_t MAX_CHUNK_SIZE;
    static const size_t CHUNKED_SIZE_LINE_MAX_LENGTH;

    RequestParser(HTTPRequest & request, HTTPError & _error, ElementParser & _element_parser);

    // Request managment
    void append(uint8_t const * str, ssize_t size);
    void new_request();
    parsing_status get_status() const;
    
    // Parsing functions
    void parse_first_line();
    void parse_header_line();
    void parse_body();
    void parse_chunked_size();
    void parse_chunked_body();
    void parse_method();
    void parse_uri();
    void parse_protocol();

    bool test_first_line();
    bool test_chunk_size();
    bool test_chunk_body();
    bool test_body();
    bool test_trailer_line();
    bool test_header_line();

    // Debug
    void dump_remainder() const;
    std::string const get_remainder() const;

private:
    struct wsHeaders {
        std::string name;
        void (RequestParser::*parser_f)(std::string & value);
    };

    static const wsHeaders headers[];

    HTTPRequestBuffer _buffer;

    HTTPRequest & _request;
    HTTPError & _error;
    ElementParser &  _element_parser;

    parsing_status _status;
    int _empty_skip_count;
    bool _processing;
    size_t _header_field_count;
    size_t _trailer_field_count;
    size_t _chunk_length;

    std::string::iterator _aux_token_start, _aux_token_end, _begin, _end;
    std::string _aux_line;

    void percentage_decode(std::string & str);

    void normalize_path(std::string & str);
    void replace_percentage(std::string::iterator & it, std::string & str);

    void get_path(std::string::iterator & token_begin, std::string::iterator & token_end);
    void get_hier_part(std::string::iterator & token_begin, std::string::iterator & token_end); 
    void get_query(std::string::iterator & token_begin, std::string::iterator & token_end);
    void get_fragment(std::string::iterator & token_begin, std::string::iterator & token_end);
    void get_schema(std::string::iterator & token_begin, std::string::iterator & token_end);
    bool has_authority(std::string::iterator & token_begin, std::string::iterator & token_end) const;

    void process_headers();
    void parse_host_field(std::string & value);
    void parse_content_length_field(std::string & value);
    void parse_transfer_encoding_field(std::string & value);
};