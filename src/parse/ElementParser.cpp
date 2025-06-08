#include "ElementParser.hpp"

ElementParser::ElementParser(ErrorContainer & error_container)
:_error_container(error_container){}

void ElementParser::parse_method(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, HTTPMethod & method)
{
    method = method::str_to_method(std::string(begin, end));
    if (method != NOMETHOD)
        return ;
    _error_container.put_error("method: " + std::string(begin, end));
    while (begin != end)
    {
        if (!parse::is_alpha(*begin))
            _error_container.put_error("Method, unexpected character", source_line, begin);
        begin ++;
    }
}

void ElementParser::parse_protocol(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & protocol)
{
    protocol = std::string(begin, end);
    wss::to_upper(protocol);
    while (begin != end)
    {
        if (!parse::is_protocol_char(*begin))
            _error_container.put_error("protocol, unexpected character", source_line, begin);
        begin ++;
    }
}

void ElementParser::parse_path(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & path)
{
    path = std::string(begin, end);
    while (begin != end)
    {
        if (*begin != '/' && !parse::is_pchar(*begin))
            _error_container.put_error("path, unexpected character", source_line, begin);
        begin ++;
    }
    normalize_path(path);
    percentage_decode(path);
}

void ElementParser::parse_query(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & query)
{
    query = std::string(begin, end);
    while (begin != end)
    {
        if (!parse::is_query_char(*begin))
            _error_container.put_error("query, unexpected character", source_line, begin);
        begin ++;
    }
    percentage_decode(query);
}

void ElementParser::parse_field_value(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & value)
{
    value = std::string(begin, end);

    while (begin != end)
    {
        if (!parse::is_field_value_char(*begin))
            _error_container.put_error("field value, unexpected character");
        begin ++;
    }
    parse::sanitize_header_value(value.begin(), value.end());
}

void ElementParser::parse_field_name(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & name)
{
    name = std::string(begin, end);

    while (begin != end)
    {
        if (!parse::is_token_char(*begin))
            _error_container.put_error("field value, unexpected character");
        begin ++;
    }
    wss::to_lower(name);
}


void ElementParser::parse_fragment(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & fragment)
{
    fragment = std::string(begin, end);
    _error_container.put_warning("fragment present, ignored", source_line, begin - 1);
    while (begin != end)
    {
        if (!parse::is_fragment_char(*begin))
            return _error_container.put_warning("fragment, unexpected character", source_line, begin);
        begin++;
    }
}

void ElementParser::parse_host(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & host)
{
    host = std::string(begin, end);
    if (*begin == '[')
    {
        _error_container.put_error("host, only IPv4 and regular host supported", source_line, begin);
        begin ++;
    }
    while (begin != end)
    {
        if (!parse::is_host_char(*begin))
            _error_container.put_error("host, unexpected character", source_line, begin);
        begin ++;
    }
    percentage_decode(host);
}

void ElementParser::parse_port(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, int & port)
{
    port = 0;

    while (begin != end)
    {
        if (!parse::is_digit(*begin))
            return _error_container.put_error("port, unexpected character", source_line, begin);
        port = port * 10 + *begin - '0';
        if (port >= 65535)
            return _error_container.put_error("port, too big (max: 65535 for TCP)");
        begin ++;
    }
}

void ElementParser::parse_content_length_field(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, int & length)
{
    length = 0;
    while (begin != end)
    {
        if (!parse::is_digit(*begin))
            return _error_container.put_error("content-length, unexpected character", source_line, begin);
        length = length * 10 + *begin - '0';
        if (length >= 10000000)
            return _error_container.put_error("content-length, too big (max: 10000000)");
        begin ++;
    }
}


void ElementParser::parse_schema(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & schema)
{
    schema = std::string(begin, end);
    while (begin != end)
    {
        if (!parse::is_alpha(*begin))
            _error_container.put_error("schema, unexpected character", source_line, begin);
        begin ++;
    }
}

void ElementParser::parse_field(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, FieldSection & fields)
{}

void ElementParser::replace_percentage(std::string::iterator & it, std::string & str)
{
    if (it + 2 >= str.end() || !parse::is_hexa_char(*(it + 1)) || !parse::is_hexa_char(*(it + 2)))
        return _error_container.put_error("percentage encoding", str, it++);
    *it = parse::hex_to_byte(*(it + 1)) * 16 + parse::hex_to_byte(*(it + 2));
    it = str.erase(it + 1, it + 3);
}

void ElementParser::percentage_decode(std::string & str)
{
    for (std::string::iterator it = str.begin(); it != str.end();)
        if (*it == '%')
            replace_percentage(it, str);
        else
            it ++;
}

void ElementParser::normalize_path(std::string & str)
{
    std::string::iterator it = str.begin();
    std::string out = "";

    while (it != str.end())
    {  
        while (std::distance(it, str.end()) >= 2 && wss::str_equal(it, 2, "//"))
            it ++;
        if (std::distance(it, str.end()) >= 3 && wss::str_equal(it, 3, "../"))
            it += 3;
        else if (std::distance(it, str.end()) >= 2 && wss::str_equal(it, 2, "./"))
            it += 2;

        else if (std::distance(it, str.end()) >= 3 && wss::str_equal(it, 3, "/./"))
            it += 2;
        else if (std::distance(it, str.end()) == 2 && wss::str_equal(it, "/."))
        {
            it ++;
            *it = '/';
        }

        else if (std::distance(it, str.end()) >= 4 && wss::str_equal(it, 4, "/../"))
        {
            it += 3;
            wss::remove_uri_segment(out);
        }
        else if (std::distance(it, str.end()) == 3 && wss::str_equal(it, "/.."))
        {
            it += 2;
            *it = '/';
            wss::remove_uri_segment(out);
        }

        else if (wss::str_equal(it, ".."))
            it +=2;
        else if (wss::str_equal(it, "."))
            it ++;

        else
            it = wss::move_uri_segment(out, str, it);
    }
    if (out.empty())
        out = "/";
    str = out;
}
