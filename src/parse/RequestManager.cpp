#include "RequestManager.hpp"

RequestManager::RequestManager(HTTPRequest & request, SysBufferFactory::sys_buffer_type type, int fd)
:_validator(_request, _error),
_element_parser(_error),
_request_parser(_request, _error, _element_parser),
_sys_buffer(SysBufferFactory::get_buffer(type, fd)),
_request(request){}

RequestManager::~RequestManager(){delete _sys_buffer;};

void RequestManager::process()
{
    bool parse = true;
    bool has_read = false;

    while (_error.status() == OK && !request_done())
    {
        switch (_request_parser.get_status())
        {
            case RequestParser::PRS_FIRST_LINE:
                parse = _request_parser.test_first_line();
                if (parse)
                {
                    _request_parser.parse_first_line();
                    if (_error.status() == OK) _validator.validate_first_line(_request);
                }
                break;
            case RequestParser::PRS_HEADER_LINE:
                parse = _request_parser.test_header_line();
                if (parse)
                    _request_parser.parse_header_line();
                if (_error.status() == OK && _request_parser.get_status() != RequestParser::PRS_HEADER_LINE)
                    _validator.validate_headers(_request, _request.headers);
                break ;
            case RequestParser::PRS_BODY:
                parse = _request_parser.test_body();
                if (parse)
                {
                    _request_parser.parse_body();
                    if (_error.status() == OK) _validator.validate_body(_request.body);
                }
                break ;
            case RequestParser::PRS_CHUNKED_SIZE:
                parse = _request_parser.test_chunk_size();
                if (parse)
                    _request_parser.parse_chunked_size();
                break ;
            case RequestParser::PRS_CHUNKED_BODY: // TODO: Validate body?
                parse = _request_parser.test_chunk_body();
                if (parse)
                    _request_parser.parse_chunked_body();
                break ;            
            case RequestParser::PRS_TRAILER_LINE: parse = _request_parser.test_trailer_line(); break ;
            case RequestParser::PRS_DONE: parse = false; break ;  
            default: throw new std::runtime_error("Code error reading Request Parser status");
        }
        if (!parse && _error.status() == OK && !request_done())
        {
            if (has_read)
                break ;
            has_read = true;
            ssize_t read_size = _sys_buffer->read(_read_buffer, _READ_BUFFER_SIZE);
            if (read_size <= 0)
                break ;
            _request_parser.append(_read_buffer, read_size);
        }
    }
}

bool RequestManager::request_done()
{
    return _request_parser.get_status() == RequestParser::PRS_DONE;
}

void RequestManager::new_request()
{
    _error.set("", OK); _request.reset(); _request_parser.new_request();
}

HTTPError & RequestManager::gerError(){return _error;}