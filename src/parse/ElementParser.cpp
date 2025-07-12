#include "ElementParser.hpp"
#include "RequestParser.hpp"
#include "HTTPError.hpp"

ElementParser::ElementParser(HTTPError & error):_error(error){}

void ElementParser::parse_method(std::string::iterator & begin, std::string::iterator & end, HTTPMethod & method)
{
    std::string method_str(begin, end);
    wss::to_upper(method_str);
    method = method::str_to_method(method_str);

    if (method != NOMETHOD)
        return ;
    while (begin != end)
    {
        if (!parse::is_alpha(*begin))
            return _error.set("Method, unexpected character", BAD_REQUEST);
        begin ++;
    }
}

void ElementParser::parse_protocol(std::string::iterator & begin, std::string::iterator & end, std::string & protocol)
{
    protocol = std::string(begin, end);
    wss::to_upper(protocol);
    while (begin != end)
    {
        if (!parse::is_protocol_char(*begin))
            return _error.set("Protocol, unexpected character", BAD_REQUEST);
        begin ++;
    }
}

void ElementParser::parse_path(std::string::iterator & begin, std::string::iterator & end, std::string & path)
{
    path = std::string(begin, end);
    while (begin != end)
    {
        if (*begin != '/' && !parse::is_pchar(*begin))
            return _error.set("Path, unexpected character", BAD_REQUEST);
        begin ++;
    }
    normalize_path(path);
    percentage_decode(path);
}

void ElementParser::parse_query(std::string::iterator & begin, std::string::iterator & end, std::string & query)
{
    query = std::string(begin, end);
    while (begin != end)
    {
        if (!parse::is_query_char(*begin))
            return _error.set("Query, unexpected character", BAD_REQUEST);
        begin ++;
    }
    percentage_decode(query);
}

void ElementParser::parse_field_value(std::string::iterator & begin, std::string::iterator & end, std::string & value)
{
    value = std::string(begin, end);

    while (begin != end)
    {
        if (!parse::is_field_value_char(*begin))
            return _error.set("field value, unexpected character", BAD_REQUEST);
        begin ++;
    }
    parse::sanitize_header_value(value.begin(), value.end());
}

void ElementParser::parse_field_token(std::string::iterator & begin, std::string::iterator & end, std::string & name)
{
    name = std::string(begin, end);

    while (begin != end)
    {
        if (!parse::is_token_char(*begin))
            return _error.set("field token, unexpected character", BAD_REQUEST);
        begin ++;
    }
    wss::to_lower(name);
}


void ElementParser::parse_fragment(std::string::iterator & begin, std::string::iterator & end, std::string & fragment)
{
    fragment = std::string(begin, end);
    while (begin != end)
    {
        if (!parse::is_fragment_char(*begin))
            return _error.set("URL, unexpected character", BAD_REQUEST);
        begin++;
    }
}

void ElementParser::parse_host(std::string::iterator & begin, std::string::iterator & end, std::string & host)
{
    host = std::string(begin, end);
    if (*begin == '[')
    {
        return _error.set("Host, the only IP protocol supported is IPv4", BAD_REQUEST);
        begin ++;
    }
    while (begin != end)
    {
        if (!parse::is_host_char(*begin))
            return _error.set("Host, unexpected character", BAD_REQUEST);
        begin ++;
    }
    percentage_decode(host);
    wss::to_lower(host);
}

void ElementParser::parse_port(std::string::iterator & begin, std::string::iterator & end, int & port)
{
    port = 0;

    while (begin != end)
    {
        if (!parse::is_digit(*begin))
            return _error.set("Port, unexpected character", BAD_REQUEST);
        port = port * 10 + *begin - '0';
        if (port > 65535)
            return _error.set("Port, too big (max: 65535 for TCP)", BAD_REQUEST);
        begin ++;
    }
}

void ElementParser::parse_content_length_field(std::string::iterator & begin, std::string::iterator & end, int & length)
{
    length = 0;
    while (begin != end)
    {
        if (!parse::is_digit(*begin))
            return _error.set("Header Content-Length, unexpected character", BAD_REQUEST);
        length = length * 10 + *begin - '0';
        if (static_cast<size_t>(length) >= RequestParser::MAX_CONTENT_LENGTH)
            return _error.set("Header Content-Length, too large", CONTENT_TOO_LARGE);
        begin ++;
    }
}

// End and begin point to  '"'
void ElementParser::parse_dquote_string(std::string::iterator & begin, std::string::iterator & end, std::string & str)
{
    begin ++;
    str.reserve(std::distance(begin, end));

    while (begin != end)
    {
        if (*begin == '\\')
        {
            if (begin + 1 == end || !parse::is_quoted_pair_char(*(begin + 1)))
                return _error.set("quoted string, unexpected character", BAD_REQUEST);
            str.push_back(*(begin + 1));
            begin += 2;
        }
        else 
        {
            if (!parse::is_qdtext_char(*begin))
                return _error.set("quoted string, unexpected character", BAD_REQUEST);
            str.push_back(*begin);
            begin ++;
        }
    }
}

void ElementParser::parse_comma_separated_values(std::string::iterator & begin, std::string::iterator & end, std::vector<Coding> & values)
{
    while (1)
    {
        Coding csf;
        std::string::iterator head;

        // Get name
        head = wss::skip_until(begin, end, ",; ");
        parse_field_token(begin, head, csf.name);
        if (csf.name.empty())
            return _error.set("Comma separated field, empty field name", BAD_REQUEST);
        
        // Parse params
        begin = wss::skip_ascii_whitespace(head, end);
        while (begin != end && *begin == ';')
        {
            std::string param_name, param_value;
        
            // Param name
            begin = wss::skip_ascii_whitespace(begin + 1, end);
            head = wss::skip_until(begin, end, "= \t");
            parse_field_token(begin, head, param_name);
            if (param_name.empty())
                return _error.set("comma separated field, empty parameter name", BAD_REQUEST);
     
            // Parse value (Skip to value)
            begin = wss::skip_ascii_whitespace(head, end);
            if (begin != end && *begin != '=')
                return _error.set("Comma separated parameter, unexpected character", BAD_REQUEST);
            if (begin == end || begin + 1 == end)
                return _error.set("Comma separated parameter, empty parameter value", BAD_REQUEST);
            begin = wss::skip_ascii_whitespace(begin + 1, end);

            // Parse value
            if (*begin == '"')
            {
                head = wss::skip_until_dquoted_string_end(begin + 1, end);
                if (head == end)
                    return _error.set("Comma separated parameter, closing dquote missing", BAD_REQUEST);
                parse_dquote_string(begin, head, param_value);

                head ++; // Skip the last '"'
            }
            else 
            {
                head = wss::skip_until(begin, end, ", \t");
                parse_field_token(begin, head, param_value);
            }
            if (param_value.empty())
                return _error.set("Comma separated parameter, empty parameter value", BAD_REQUEST);

            // Put value and push begin iterator
            csf.parameters.push_back(std::pair<std::string, std::string>(param_name, param_value));
            begin = wss::skip_ascii_whitespace(head, end);
        }

        values.push_back(csf);
    
        // End of current field
        if (begin != end && *begin == ',')
        {
            begin = wss::skip_ascii_whitespace(begin + 1, end);
            continue ;
        }
        else if (begin != end)
            return _error.set("Comma separated field, unexpected character", BAD_REQUEST);
        break ;
    } 
}


void ElementParser::parse_schema(std::string::iterator & begin, std::string::iterator & end, std::string & schema)
{
    schema = std::string(begin, end);
    while (begin != end)
    {
        if (!parse::is_alpha(*begin))
            return _error.set("Schema, unexpected character", BAD_REQUEST);
        begin ++;
    }
    wss::to_lower(schema);
}

void ElementParser::replace_percentage(std::string::iterator & it, std::string & str)
{
    if (it + 2 >= str.end() || !parse::is_hexa_char(*(it + 1)) || !parse::is_hexa_char(*(it + 2)))
        return _error.set("Percentage encoding, unexpected character", BAD_REQUEST);
    *it = parse::hex_to_byte(*(it + 1)) * 16 + parse::hex_to_byte(*(it + 2));
    it = str.erase(it + 1, it + 3);
}

void ElementParser::percentage_decode(std::string & str)
{
    for (std::string::iterator it = str.begin(); it != str.end();)
        if (_error.status() != OK)
            return;
        else if (*it == '%')
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
