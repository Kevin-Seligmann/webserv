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
    
    std::string s_method;
    get_first_line_words(_line, s_method, _request.uri.raw, _request.protocol);
    _request.method = method::str_to_method(s_method);
    _validator.validate_method(_request.method);
    _validator.validate_protocol(_request.protocol);
    _validator.validate_uri(_request.uri);
    _status = HEADERS;
}

void RequestParser::get_first_line_words(std::string const & line, std::string & method, std::string & uri, std::string & protocol)
{
    std::string::const_iterator it = line.begin();

    it = wss::copy_method(method, it, line.end());
    if (it == line.end() || *it != ' ')
    {
        throw std::runtime_error(
            "Invalid request: Malformed method\n"
            "  " + line + "\n"
            "  " + std::string(std::distance(line.begin(), it), ' ') + "^\n"
        );
    }

    it = wss::copy_uri_token(uri, it, line.end());
    if (it == line.end() || *it != ' ')
    {
        throw std::runtime_error(
            "Invalid request: Malformed URI\n"
            "  " + line + "\n"
            "  " + std::string(std::distance(line.begin(), it), ' ') + "^\n"
        );
    }
    
    it = wss::copy_protocol(protocol, it, line.end());
    it = wss::skip_ascii_whitespace(it, line.end());
    if (it != line.end())
    {
        throw std::runtime_error(
            "Invalid request: Malformed protocol\n"
            "  " + line + "\n"
            "  " + std::string(std::distance(line.begin(), it), ' ') + "^\n"
        );
    }
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
