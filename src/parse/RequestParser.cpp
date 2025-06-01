#include "RequestParser.hpp"

const std::string RequestParser::ILLEGAL_CHAR = std::string("\0", 1);

RequestParser::RequestParser(HTTPRequest & request)
:_request(request), 
_status(FIRST_LINE),
_empty_skip_count(0)
{}

bool RequestParser::done() const
{
    return _status == DONE;
}

void RequestParser::new_request()
{
    _status = FIRST_LINE;
    _buffer.new_request();
    _request.reset();
    _empty_skip_count = 0;
}

bool RequestParser::illegal(char c) const
{
    return ILLEGAL_CHAR.find(c) != std::string::npos;
}

void RequestParser::sanitize(std::string & str) const
{
    for (std::string::iterator it = str.begin(); it != str.end(); it ++)
        if (*it == '\r')
            *it = ' ';
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
            throw std::runtime_error("Invalid request: More than one empty line before request");
        return ;
    }
    
    get_first_line_words();
    _validator.validate_method(_request.method);
    _validator.validate_protocol(_request.protocol);
    _validator.validate_uri(_request.uri);
    _status = HEADERS;
}

void RequestParser::get_first_line_words()
{
    std::string::const_iterator it = _line.begin();

    it = get_method(it, _line.end());
    it = get_uri(it, _line.end());
    it = get_protocol(it, _line.end());
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
    sanitize(_line);
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

void RequestParser::parse_body(){_status = DONE;}

void RequestParser::dump_remainer() const
{
    Logger::getInstance() << "Buffer remainer:\n" << std::string(_buffer.cbegin(), _buffer.cend()) << "$\n";
}

void RequestParser::malformed(std::string const & what, std::string::const_iterator place)
{
    throw std::runtime_error(
        "Invalid request: Malformed " + what + ":\n"
        "   " + _line + "\"\n"
        "   " + std::string(std::distance((std::string::const_iterator) _line.begin(), place), ' ') + "^"
    );
}

std::string::const_iterator RequestParser::get_method(std::string::const_iterator begin, std::string::const_iterator end)
{
    std::string::const_iterator token_start = wss::skip_ascii_whitespace(begin, end);
    std::string::const_iterator token_end = wss::skip_http_token(token_start, end);
    if (token_end == _line.end() || *token_end != ' ')
        malformed("method", token_end);
    _request.method = method::str_to_method(std::string(token_start, token_end));
    return token_end;
}

std::string::const_iterator RequestParser::get_uri(std::string::const_iterator begin, std::string::const_iterator end)
{
    std::string::const_iterator token_start = wss::skip_ascii_whitespace(begin, end);
    std::string::const_iterator token_end = wss::skip_uri_token(token_start, end);
    if (token_end == _line.end() || *token_end != ' ')
        malformed("uri", token_end);
    _request.uri.raw = std::string(token_start, token_end);
    return token_end;
}

std::string::const_iterator RequestParser::get_protocol(std::string::const_iterator begin, std::string::const_iterator end)
{
    std::string::const_iterator token_start = wss::skip_ascii_whitespace(begin, end);
    std::string::const_iterator token_end = wss::skip_protocol_token(token_start, end);
    _request.protocol = std::string(token_start, token_end);
    token_end = wss::skip_ascii_whitespace(token_end, _line.end());
    if (token_end != _line.end())
        malformed("first line", token_end);
    return token_end;
}