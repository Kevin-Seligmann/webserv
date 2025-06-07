#include "RequestParser.hpp"

RequestParser::RequestParser(HTTPRequest & request, ErrorContainer & error_container, ElementParser & element_parser, RequestValidator & validator)
:_request(request), 
_error_container(error_container),
_element_parser(element_parser),
_validator(validator),
_status(FIRST_LINE),
_empty_skip_count(0)
{}

bool RequestParser::done() const {return _status == DONE;}

void RequestParser::parse_body(){_status = DONE;}

void RequestParser::dump_remainder() const
{
    Logger::getInstance() << "Buffer remainder:\n" << std::string(_buffer.cbegin(), _buffer.cend()) << "$\n";
}

void RequestParser::append(uint8_t *str, ssize_t size) {_buffer.append(str, size);}

void RequestParser::new_request()
{
    _empty_skip_count = 0;
    _status = FIRST_LINE;
    _buffer.new_request();
}

void RequestParser::process()
{
    _processing = true;
    while (_processing && _status != DONE)
    {
        switch (_status)
        {
            case FIRST_LINE: parse_first_line(); break ;
            case HEADERS: parse_header_line(); break ;
            case BODY: parse_body() ; break ;
            case DONE: throw std::runtime_error("CODE ERROR: Must initialize a new request");
        }
    }
}


void RequestParser::parse_first_line()
{
    _processing = _buffer.get_crlf_line(_line);
    if (!_processing)
        return ;
    if (_line.empty())
    {
        _empty_skip_count ++;
        if (_empty_skip_count > 1)
            _error_container.put_error("Only one empty line is allowed before request");
        return ;
     }

    parse::first_line_sanitize(_line);
    get_method();
    parse_uri();
    get_protocol();
    if (wss::skip_whitespace(_token_end, _line.end()) != _line.end())
        _error_container.put_error("first line, unexpected character", _line, _token_end);

    _validator.validate_uri(_request.uri);
    _validator.validate_method(_request.method);
    _validator.validate_protocol(_request.protocol);
    _status = HEADERS;
}

void RequestParser::get_method()
{
    _token_start = _line.begin();
    if (*_token_start == ' ')
    {
        _error_container.put_warning("First line, space before method", _line, _token_start);
        _token_start = wss::skip_whitespace(_line.begin(), _line.end());
    }

    _token_end = wss::skip_until(_token_start, _line.end(), " ");
    _element_parser.parse_method(_token_start, _token_end, _line, _request.method);
    _token_start = _token_end;
}

void RequestParser::parse_header_line()
{
    _processing = _buffer.get_crlf_line(_line);
    if (!_processing)
        return ;
    
    if (_line.empty())
    {
        _validator.validate_headers(_request.headers);
        _status = BODY;
        return ;
    }

    _token_start = _line.begin();
    _token_end = wss::skip_http_token(_token_start, _line.end());
    std::string name(_token_start, _token_end);
    if (_token_end != _line.end() && *_token_end != ':')
    {
        _error_container.put_error("field name, unexpected character", _line, _token_end);
        while (_token_end != _line.end() && *_token_end != ':')
            _token_end ++; 
    }
    if (_token_end != _line.end())
        _token_end ++;

    _token_start = wss::skip_ascii_whitespace(_token_end, _line.end());
    if (_token_start != _line.end())
        _token_end = wss::skip_ascii_whitespace_r(_line.end(), _token_start);
    std::string value(_token_start, _token_end);
    parse::sanitize_header_value(value.begin(), value.end());
 
    put_header_value(name, value);
}

void RequestParser::put_header_value(std::string const & name, std::string const & value)
{
    if (name.empty())
        _error_container.put_error("field name, empty", _line, _token_end);
    if (value.empty())
        _error_container.put_error("field value, empty", _line, _token_end);
    if (name.empty() || value.empty())
        return ;
    _request.headers.put(name, value);
}

bool RequestParser::has_authority() const
{
    return _token_start + 1 < _line.end() && *_token_start == '/' && *(_token_start + 1) == '/';
}

void RequestParser::get_hier_part()
{ 
    _token_start += 2; // Skip '//'

    // Find and skip userinfo if present
    _token_end = wss::skip_until(_token_start, _line.end(), "@");
    if (_token_end != _line.end() && *_token_end == '@')
    {
        _error_container.put_error("authority, userinfo '@' is deprecated", _line, _token_end);
        _token_start = _token_end + 1;
    }
    if (_token_start == _line.end())
        return _error_container.put_error("host, not found");

    // Find host (Until path or end)
    _token_end = wss::skip_until(_token_start, _line.end(), " :?#/");
    _element_parser.parse_host(_token_start, _token_end, _line, _request.uri.host);
    _token_start = _token_end;

    // Find port
    if (_token_end != _line.end() && *_token_end == ':')
    {
        _token_start ++;
        _token_end = wss::skip_until(_token_start, _line.end(), " ?#/");
        _element_parser.parse_port(_token_start, _token_end, _line, _request.uri.port);
        _token_start = _token_end;
    }

    if (_token_end != _line.end() && *_token_end == '/')
        get_path();
}

void RequestParser::parse_uri()
{
    // Skip to start of URI
    if (_token_start == _line.end() && _token_start + 1 == _line.end() && *_token_start == ' ' && *(_token_start + 1) == ' ')
        _error_container.put_warning("URI, extra whitespace", _line, _token_start);
    _token_start = wss::skip_whitespace(_token_start, _line.end());
    if (_token_start == _line.end())
        return _error_container.put_error("URI not found");

    if (_token_start != _line.end() && *_token_start == '/')
        get_path();
    else if (_token_start != _line.end())
    {
        get_schema();
        if (has_authority())
            get_hier_part();
        else
            get_path();
    }
    if (_token_start != _line.end() && *_token_start == '?')
        get_query();
    if (_token_start != _line.end() && *_token_start == '#')
        get_fragment();
}

void RequestParser::get_path()
{
    _token_end = wss::skip_until(_token_start, _line.end(), " ?#");
    _element_parser.parse_path(_token_start, _token_end, _line, _request.uri.path);
    _token_start = _token_end;
}


void RequestParser::get_schema()
{
    _token_end = wss::skip_until(_token_start, _line.end(), ":");
    if (_token_end == _line.end())
        return _error_container.put_error("URI schema, separator ':' not found");
    _element_parser.parse_schema(_token_start, _token_end, _line, _request.uri.schema);
    _token_start = _token_end + 1;
}

void RequestParser::get_query()
{
    _token_start ++;
    _token_end = wss::skip_until(_token_start, _line.end(), " #");
    _element_parser.parse_query(_token_start, _token_end, _line, _request.uri.query);
    _token_start = _token_end;
}

void RequestParser::get_fragment()
{
    _token_start ++;
    _token_end = wss::skip_until(_token_start, _line.end(), " ");
    _element_parser.parse_fragment(_token_start, _token_end, _line, _request.uri.fragment);
    _token_start = _token_end;
}

void RequestParser::get_protocol()
{
    // Skip to start of Protocol
    if (_token_start == _line.end() && _token_start + 1 == _line.end() && *_token_start == ' ' && *(_token_start + 1) == ' ')
        _error_container.put_warning("protocol, extra whitespace", _line, _token_start);
    _token_start = wss::skip_whitespace(_token_start, _line.end());
    if (_token_start == _line.end())
        return _error_container.put_error("protocol not found");

    _token_end = wss::skip_until(_token_start, _line.end(), " ");
    _element_parser.parse_protocol(_token_start, _token_end, _line, _request.protocol);

    if (_token_end == _line.end())
        return ;
    _token_end = wss::skip_whitespace(_token_end, _line.end());
    if (_token_end != _line.end())
        _error_container.put_error("request line, extra content", _line, _token_end);
    else
        _error_container.put_warning("request line, extra whitespace after protocol");
} 
