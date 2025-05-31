#include "RequestParser.hpp"

const std::string RequestParser::ILLEGAL_CHAR = std::string("\0\r", 2);

RequestParser::RequestParser(HTTPRequest & request)
:_request(request), 
_status(FIRST_LINE)
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
    bool rta = true;
    std::string line;

    while (rta && _status != DONE)
    {
        switch (_status)
        {
            case FIRST_LINE:
                rta = _buffer.get_crlf_line(line);
                if (rta)
                {
                    if (line.empty())
                        break ;
                     sanitize(line);
                    _request.first_line.put(line);
                    _status = HEADERS;
                }
                break;
            case HEADERS:
                rta = _buffer.get_crlf_line(line);
                if (rta)
                {
                    sanitize(line);
                    if (line.empty())
                        _status = BODY;
                    else
                        _request.headers.put(line);
                }
                break;
            case BODY:
                _status = DONE;
            break;
            case DONE:
                throw std::runtime_error("CODE ERROR: Must initialize a new request");
        }
    }
}

void RequestParser::dump_remainer() const
{
    Logger::getInstance() << "Buffer remainer:\n" << std::string(_buffer.cbegin(), _buffer.cend()) << "$\n";
}
