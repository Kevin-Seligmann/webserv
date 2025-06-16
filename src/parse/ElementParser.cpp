#include "ElementParser.hpp"
#include "RequestParser.hpp"

ElementParser::ElementParser(ErrorContainer & error_container)
:_error_container(error_container){}

void ElementParser::parse_method(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, HTTPMethod & method)
{
    std::string method_str(begin, end);
    wss::to_upper(method_str);
    method = method::str_to_method(method_str);

    if (method != NOMETHOD)
        return ;
    while (begin != end)
    {
        if (!parse::is_alpha(*begin))
            _error_container.put_error("Method, unexpected character", source_line, begin, BAD_REQUEST);
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
            _error_container.put_error("protocol, unexpected character", source_line, begin, BAD_REQUEST);
        begin ++;
    }
}

void ElementParser::parse_path(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & path)
{
    path = std::string(begin, end);
    while (begin != end)
    {
        if (*begin != '/' && !parse::is_pchar(*begin))
            _error_container.put_error("path, unexpected character", source_line, begin, BAD_REQUEST);
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
            _error_container.put_error("query, unexpected character", source_line, begin, BAD_REQUEST);
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
            _error_container.put_error("field value, unexpected character", BAD_REQUEST);
        begin ++;
    }
    parse::sanitize_header_value(value.begin(), value.end());
}

void ElementParser::parse_field_token(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & name)
{
    name = std::string(begin, end);

    while (begin != end)
    {
        if (!parse::is_token_char(*begin))
            _error_container.put_error("field token, unexpected character", source_line, end, BAD_REQUEST);
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
        _error_container.put_error("host, only IPv4 and regular host supported", source_line, begin, BAD_REQUEST);
        begin ++;
    }
    while (begin != end)
    {
        if (!parse::is_host_char(*begin))
            _error_container.put_error("host, unexpected character", source_line, begin, BAD_REQUEST);
        begin ++;
    }
    percentage_decode(host);
    wss::to_lower(host);
}

void ElementParser::parse_port(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, int & port)
{
    port = 0;

    while (begin != end)
    {
        if (!parse::is_digit(*begin))
            return _error_container.put_error("port, unexpected character", source_line, begin, BAD_REQUEST);
        port = port * 10 + *begin - '0';
        if (port >= 65535)
            return _error_container.put_error("port, too big (max: 65535 for TCP)", BAD_REQUEST);
        begin ++;
    }
}

void ElementParser::parse_content_length_field(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, int & length)
{
    length = 0;
    while (begin != end)
    {
        if (!parse::is_digit(*begin))
            return _error_container.put_error("content-length, unexpected character", source_line, begin, BAD_REQUEST);
        length = length * 10 + *begin - '0';
        if (length >= RequestParser::MAX_CONTENT_LENGTH)
            return _error_container.put_error("Content-Length header", CONTENT_TOO_LARGE);
        begin ++;
    }
}

// End and being point to  '"'
void ElementParser::parse_dquote_string(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & str)
{
    begin ++;
    str.reserve(std::distance(begin, end));

    while (begin != end)
    {
        if (*begin == '\\')
        {
            if (begin + 1 == end || !parse::is_quoted_pair_char(*(begin + 1)))
                return _error_container.put_error("quoted string, unexpected character", source_line, begin, BAD_REQUEST);
            str.push_back(*(begin + 1));
            begin += 2;
        }
        else 
        {
            if (!parse::is_qdtext_char(*begin))
                return _error_container.put_error("quoted string, unexpected character", source_line, begin, BAD_REQUEST);
            str.push_back(*begin);
            begin ++;
        }
    }
}

void ElementParser::parse_comma_separated_values(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::vector<CommaSeparatedFieldValue> & values)
{
    while (1)
    {
        CommaSeparatedFieldValue csf;

        // Get name
        end = wss::skip_until(begin, source_line.end(), ",; ");
        parse_field_token(begin, end, source_line, csf.name);
        if (csf.name.empty())
            _error_container.put_error("comma separated field, empty field name", source_line, begin, BAD_REQUEST);
        
        // Parse params
        begin = wss::skip_ascii_whitespace(end, source_line.end());
        while (begin != source_line.end() && *begin == ';')
        {
            std::string param_name, param_value;
        
            // Param name
            begin = wss::skip_ascii_whitespace(begin + 1, source_line.end());
            end = wss::skip_until(begin, source_line.end(), "= \t");
            parse_field_token(begin, end, source_line, param_name);
            if (param_name.empty())
                _error_container.put_error("comma separated field, empty parameter name", source_line, begin, BAD_REQUEST);
     
            // Parse value (Skip to value)
            begin = wss::skip_ascii_whitespace(end, source_line.end());
            if (begin != source_line.end() && *begin != '=')
                return _error_container.put_error("comma separated parameter, unexpected character", source_line, begin, BAD_REQUEST);
            if (begin == source_line.end() || begin + 1 == source_line.end())
                return _error_container.put_error("comma separated parameter, empty parameter value", source_line, begin, BAD_REQUEST);
            begin = wss::skip_ascii_whitespace(begin + 1, source_line.end());

            // Parse value
            if (*begin == '"')
            {
                end = wss::skip_until_dquoted_string_end(begin + 1, source_line.end());
                if (end == source_line.end())
                    return _error_container.put_error("comma separated parameter, closing dquote missing", source_line, end, BAD_REQUEST);
                parse_dquote_string(begin, end, source_line, param_value);

                end ++; // Skip the last '"'
            }
            else 
            {
                end = wss::skip_until(begin, source_line.end(), ", \t");
                parse_field_token(begin, end, source_line, param_value);
            }
            if (param_value.empty())
                return _error_container.put_error("comma separated parameter, empty parameter value", source_line, begin, BAD_REQUEST);

            // Put value and push begin iterator
            csf.parameters.push_back(std::pair<std::string, std::string>(param_name, param_value));
            begin = wss::skip_ascii_whitespace(end, source_line.end());
        }

        values.push_back(csf);
    
        // End of current field
        if (begin != source_line.end() && *begin == ',')
        {
            begin = wss::skip_ascii_whitespace(begin + 1, source_line.end());
            continue ;
        }
        else if (begin != source_line.end())
            return _error_container.put_error("comma separated field, unexpected character", source_line, begin, BAD_REQUEST);
        break ;
    } 
}


void ElementParser::parse_schema(std::string::const_iterator & begin, std::string::const_iterator & end, std::string const & source_line, std::string & schema)
{
    schema = std::string(begin, end);
    while (begin != end)
    {
        if (!parse::is_alpha(*begin))
            _error_container.put_error("schema, unexpected character", source_line, begin, BAD_REQUEST);
        begin ++;
    }
    wss::to_lower(schema);
}

void ElementParser::replace_percentage(std::string::iterator & it, std::string & str)
{
    if (it + 2 >= str.end() || !parse::is_hexa_char(*(it + 1)) || !parse::is_hexa_char(*(it + 2)))
        return _error_container.put_error("percentage encoding", str, it++, BAD_REQUEST);
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
