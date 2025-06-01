#include "RequestParser.hpp"

const std::string RequestParser::ILLEGAL_CHAR = std::string("\0\r", 2);

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
        if (illegal(*it))
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
    
    sanitize(_line);
    std::stringstream os(_line);
    std::string s_method;

    os >> s_method >> _request.uri.raw >> _request.protocol;
    if (os >> _line)
        throw std::runtime_error("Invalid request: Extra content at first line: " + _line);
    
    wss::to_upper(_request.protocol);
    wss::to_upper(s_method);
    _request.method = method::str_to_method(s_method);
    _validator.validate_method(_request.method);
    _validator.validate_protocol(_request.protocol);
    _validator.validate_uri(_request.uri);
    _status = HEADERS;
}


void RequestParser::parse_header_line()
{
    _processing = _buffer.get_crlf_line(_line);
    if (_processing)
    {
        if (_line.empty())
        {
            _status = BODY;
            return ;
        }
        sanitize(_line);
        _request.headers.put(_line);
    }
}

void RequestParser::parse_body(){_status = DONE;}

void RequestParser::dump_remainer() const
{
    Logger::getInstance() << "Buffer remainer:\n" << std::string(_buffer.cbegin(), _buffer.cend()) << "$\n";
}
