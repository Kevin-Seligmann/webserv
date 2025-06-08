#pragma once

#include <string>
#include "HTTPRequest.hpp"
#include "FieldSection.hpp"
#include "ErrorContainer.hpp"
#include "ParsingUtil.hpp"
#include "StringUtil.hpp"

class ElementParser {
public:
    ElementParser(ErrorContainer & error_container);

    void parse_method(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, HTTPMethod & method);
    void parse_protocol(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & protocol);
    void parse_field(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, FieldSection & fields);
    void parse_path(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & path);
    void parse_host(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & host);
    void parse_port(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, int & port);
    void parse_schema(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & schema);
    void parse_fragment(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & query);
    void parse_query(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & fragment);
    void parse_field_value(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & value);
    void parse_field_name(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & name);
    void parse_content_length_field(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, int & length);

private:
    ErrorContainer & _error_container;

    void normalize_path(std::string & str);
    void percentage_decode(std::string & str);
    void replace_percentage(std::string::iterator & it, std::string & str);
};