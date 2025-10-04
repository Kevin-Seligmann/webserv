#include "RequestManager.hpp"

RequestManager::RequestManager(HTTPRequest & request, HTTPError & error, SysBufferFactory::sys_buffer_type type, int fd, StreamRequest & stream_request)
:_request(request), _error(error),
_validator(_request, _error),
_element_parser(_error),
_request_parser(_request, _error, _element_parser),
_sys_buffer(SysBufferFactory::get_buffer(type, fd)),
_stream_request(stream_request)
{
    _processing_type = STANDARD;
}

RequestManager::~RequestManager(){delete _sys_buffer;};

void RequestManager::process()
{         
    bool parse = true;
    bool has_read = false;

    if (_processing_type == STREAM)
    {
        process_stream();
        return ;
    }
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
                {
                    _validator.validate_headers(_request, _request.headers);
                   _stream_request.streaming_active = wss::isCgiRequest(_request.get_path()) &&  !_request.headers.transfer_encodings.empty() && _request.headers.transfer_encodings.back().name == "chunked";
                }
                break ;
            case RequestParser::PRS_BODY:
                _processing_type = STANDARD;
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
                {
                    _request_parser.parse_chunked_size();
                    chunk_size = _request_parser.get_chunk_length();
                    if (chunk_size > 0 && _stream_request.streaming_active)
                    {
                        set_streaming(has_read);
                        return ;
                    }
                }
                break ;
            case RequestParser::PRS_CHUNKED_BODY:
                if (_stream_request.streaming_active)
                    parse = _request_parser.test_chunk_newline();
                else
                {
                    parse = _request_parser.test_chunk_body();
                    if (parse)
                        _request_parser.parse_chunked_body();
                }
                break ;   
            case RequestParser::PRS_TRAILER_LINE: parse = _request_parser.test_trailer_line(); break ;
            case RequestParser::PRS_DONE: 
                parse = false;
                _stream_request.request_read_finished = true; 
                Logger::getInstance() << "TOTAL INPUT BODY SIZE READ " << _stream_request.request_body_size << " fd " << _sys_buffer->_fd << std::endl;
                break ;  
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

bool RequestManager::request_done() const
{
    return _request_parser.get_status() == RequestParser::PRS_DONE;
}

void RequestManager::new_request()
{
    _error.set("", OK); _request.reset(); _request_parser.new_request(); _stream_request.reset(); _error.close = false;
}

bool RequestManager::has_error() const 
{
    return _error.status() != OK;
}

bool RequestManager::close() const
{
    return (_request.headers.close_status == RCS_CLOSE || _error.close);
}

HTTPError & RequestManager::gerError(){return _error;}


/*
    The HTTPRequestBuffer of RequestParser has up to _sys_buffer size (_READ_BUFFER_SIZE) of chunked body bytes.
    
    1. Move up to CHUNK_SIZE bytes from HTTPRequestBuffer back to _sys_buffer (Guaranteed to be possible)
*/
void RequestManager::set_streaming(bool has_read)
{
    chunk_size = _request_parser.get_chunk_length();
    chunk_read = _request_parser.extract_buffer_chunk(_read_buffer, _READ_BUFFER_SIZE);
    _stream_request.get_request_buffer().set_external_buffer(_read_buffer, _READ_BUFFER_SIZE, chunk_read);
    _stream_request.set_request_read_fd(_sys_buffer->_fd);
    _stream_request.set_response_write_fd(_sys_buffer->_fd);
    _stream_request.request_body_size += chunk_size;
    _stream_request.request_body_size_appended += chunk_read;
    _processing_type = STREAM;
    if (chunk_read >= chunk_size)
        unset_streaming(has_read);
    else if (!has_read)
        process_stream();
}

/*
    1. Read into the stream buffer up to CHUNK_SIZE_REMAINING bytes or the remaining size of _read_buffer.
    2. Signal when it's over by changing _processing_type to STANDARD
    3. The request parser will be able to test that there's a newline as it should
*/
void RequestManager::process_stream()
{
    size_t remaining_chunk_size = chunk_size - chunk_read;
    ssize_t read = _stream_request.get_request_buffer().recv_external(remaining_chunk_size);
    if (read > 0)
    {
        _stream_request.request_body_size_appended += read;
        chunk_read += read;
    }
    if (chunk_read >= chunk_size)
        unset_streaming(true);
}

void RequestManager::unset_streaming(bool has_read)
{
    _processing_type = STANDARD;
    if (!has_read)
        process();
}