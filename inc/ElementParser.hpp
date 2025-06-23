#pragma once

#include <string>
#include "HTTPRequest.hpp"
#include "FieldSection.hpp"
#include "ParsingUtil.hpp"
#include "StringUtil.hpp"

class HTTPError;

class ElementParser {
public:
    ElementParser(HTTPError & _error);

    void parse_method(std::string::iterator & begin, std::string::iterator & end, HTTPMethod & method);
    void parse_protocol(std::string::iterator & begin, std::string::iterator & end, std::string & protocol);
    void parse_path(std::string::iterator & begin, std::string::iterator & end, std::string & path);
    void parse_host(std::string::iterator & begin, std::string::iterator & end, std::string & host);
    void parse_port(std::string::iterator & begin, std::string::iterator & end, int & port);
    void parse_schema(std::string::iterator & begin, std::string::iterator & end, std::string & schema);
    void parse_fragment(std::string::iterator & begin, std::string::iterator & end, std::string & query);
    void parse_query(std::string::iterator & begin, std::string::iterator & end, std::string & fragment);
    void parse_field_value(std::string::iterator & begin, std::string::iterator & end, std::string & value);
    void parse_field_token(std::string::iterator & begin, std::string::iterator & end, std::string & name);
   
    void parse_content_length_field(std::string::iterator & begin, std::string::iterator & end, int & length);
    void parse_comma_separated_values(std::string::iterator & begin, std::string::iterator & end, std::vector<Coding> & values);
    void parse_dquote_string(std::string::iterator & begin, std::string::iterator & end, std::string & str);

private:
    HTTPError & _error;

    void normalize_path(std::string & str);
    void percentage_decode(std::string & str);
    void replace_percentage(std::string::iterator & it, std::string & str);
};