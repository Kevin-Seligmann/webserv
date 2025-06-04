#include "RequestParser.hpp"

RequestParser::RequestParser(HTTPRequest & request)
:_request(request), 
_status(FIRST_LINE),
_empty_skip_count(0)
{}

bool RequestParser::done() const
{
    return _status == DONE;
}

bool RequestParser::error() const
{
    return !_validator.is_ok();
}

void RequestParser::parse_body(){_status = DONE;}

void RequestParser::dump_remainer() const
{
    Logger::getInstance() << "Buffer remainer:\n" << std::string(_buffer.cbegin(), _buffer.cend()) << "$\n";
}

void RequestParser::new_request()
{
    _validator.reset();
    _status = FIRST_LINE;
    _buffer.new_request();
    _request.reset();
    _empty_skip_count = 0;
}


void RequestParser::append(uint8_t *str, ssize_t size)
{
    _buffer.append(str, size);
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
        if (error())
        {
            _validator.log_errors();
            return ;
        }
    }
}

    void RequestParser::percentage_decode(std::string & str)
    {

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
            _validator.error("Only one empty line is allowed before request");
        return ;
    }
    
    parse::first_line_sanitize(_line);

    get_method();
    //if (_token_end == _line.end() || *_token_end != ' ')
    //    _validator.error("method, unexpected character", _line, _token_end);

    parse_uri();
    //if (_token_end == _line.end() || *_token_end != ' ')
    //    _validator.error("uri, unexpected character", _line, _token_end);

    get_protocol();

    if (wss::skip_ascii_whitespace(_token_end, _line.end()) != _line.end())
        _validator.error("first line, unexpected character", _line, _token_end);

    _validator.validate_uri(_request.uri);
    _validator.validate_method(_request.method);
    _validator.validate_protocol(_request.protocol);
    _status = HEADERS;
}

void RequestParser::get_method()
{
    _token_start = wss::skip_ascii_whitespace(_line.begin(), _line.end());
    _token_end = wss::skip_http_token(_token_start, _line.end());
    _request.method = method::str_to_method(std::string(_token_start, _token_end));
    if (_request.method == NOMETHOD)
        _validator.error("method", _line, _token_end);
    _token_start = _token_end;
}

void RequestParser::parse_header_line()
{
    _processing = _buffer.get_crlf_line(_line);
    if (!_processing)
        return ;
    
    if (_line.empty())
    {
        _status = BODY;
        return ;
    }

    parse::first_line_sanitize(_line);
    size_t separator_pos = _line.find(':');
    if (separator_pos == std::string::npos)
        throw std::runtime_error("Invalid request: Missing ':': " + _line);

    std::string name = _line.substr(0, separator_pos);
    wss::to_lower(name);
    if (!parse::is_token(name))
        throw std::runtime_error("Invalid request: Header name is not a valid token: " + name);

    std::string value = _line.substr(separator_pos + 1, std::string::npos);
    wss::trim(value);
    _request.headers.put(name, value);
}

bool RequestParser::has_authority()
{
    return _token_start + 1 < _line.end() && *_token_start == '/' && *(_token_start + 1) == '/';
}

void RequestParser::get_hier_part()
{ 
    _token_start += 2;
    _token_end = wss::skip_host_token(_token_start, _line.end());
    _request.uri.host = std::string(_token_start, _token_end);
    percentage_decode(_request.uri.host);

    _token_start = _token_end;
    if (_token_end != _line.end() && *_token_end == ':')
    {
        _token_start ++;
        _token_end = wss::skip_port_token(_token_start, _line.end());
        _request.uri.port = atoi(std::string(_token_start, _token_end).c_str());
    }

    _token_start = _token_end;
    if (_token_end != _line.end() && *_token_end == '/')
        get_absolute_path();
}

void RequestParser::parse_uri()
{
    _token_start = wss::skip_ascii_whitespace(_token_start, _line.end());
    if (_token_start != _line.end() && *_token_start == '/')
    {
        get_absolute_path();
    }
    else if (_token_start != _line.end())
    {
        get_schema();
        if (has_authority())
            get_hier_part();
        else if (_token_start != _line.end() && *_token_start == '/')
            get_path_absolute();
        else
            get_path_rootless_or_empty();
    }
    if (_token_start != _line.end() && *_token_start == '?')
        get_query();
    if (_token_start != _line.end() && *_token_start == '#')
        get_fragment();
}

void RequestParser::get_absolute_path()
{
    _token_end = wss::skip_absolute_path(_token_start, _line.end());
    _request.uri.path = std::string(_token_start, _token_end);
    percentage_decode(_request.uri.path);
    _token_start = _token_end;
}

void RequestParser::get_schema()
{
    _token_end = wss::skip_schema_token(_token_start, _line.end());
    _request.uri.schema = std::string(_token_start, _token_end);
    if (_token_end != _line.end())
    {
        if (*_token_end != ':')
            _validator.error("schema", _line, _token_end);
        _token_end ++;
    }
    _token_start = _token_end;
}

void RequestParser::get_path_absolute()
{
    get_absolute_path();
}

void RequestParser::get_path_rootless_or_empty()
{
    _token_end = wss::skip_path_rootless(_token_start, _line.end());
    _request.uri.path = std::string(_token_start,  _token_end);
    percentage_decode(_request.uri.path);
    _token_start = _token_end;
}

void RequestParser::get_query()
{
    _token_end = wss::skip_query_token(_token_start, _line.end());
    _request.uri.query = std::string(_token_start, _token_end);
    percentage_decode(_request.uri.query);
    _token_start = _token_end;
}

void RequestParser::get_fragment()
{
    _token_end = wss::skip_fragment_token(_token_start, _line.end());
    _request.uri.fragment = std::string(_token_start, _token_end);
    percentage_decode(_request.uri.fragment);
    _token_start = _token_end;
}

void RequestParser::get_protocol()
{
    _token_start = wss::skip_ascii_whitespace(_token_start, _line.end());
    _token_end = wss::skip_protocol_token(_token_start, _line.end());
    _request.protocol = std::string(_token_start, _token_end);
}