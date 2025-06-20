#include "RequestParser.hpp"

const RequestParser::wsHeaders RequestParser::headers[] = {
    {"host", &RequestParser::parse_host_field},
    {"content-length", &RequestParser::parse_content_length_field},
    {"transfer-encoding", &RequestParser::parse_transfer_encoding_field},
    {"", NULL}
};

const size_t RequestParser::URI_MAX_LENGTH = 2000;
const size_t RequestParser::FIRST_LINE_MAX_LENGTH = 8000;
const size_t RequestParser::HEADER_LINE_MAX_LENGTH = 8000;
const size_t RequestParser::MAX_HEADER_FIELDS = 100;
const size_t RequestParser::MAX_TRAILER_FIELDS = 20;
const size_t RequestParser::MAX_CONTENT_LENGTH = 1*512*1024;
const size_t RequestParser::MAX_CHUNK_SIZE = 1*512 *1024;;
const size_t RequestParser::CHUNKED_SIZE_LINE_MAX_LENGTH = 200;

RequestParser::RequestParser(HTTPRequest & request, HTTPError & error, ElementParser & element_parser, RequestValidator & validator)
:_request(request), 
_error(error),
_element_parser(element_parser),
_validator(validator),
_status(PRS_FIRST_LINE),
_empty_skip_count(0),
_header_field_count(0),
_trailer_field_count(0),
_chunk_length(0)
{}

bool RequestParser::done() const {return _status == PRS_DONE;}

void RequestParser::dump_remainder() const
{
    Logger::getInstance() << "Buffer remainder:\n" << std::string(_buffer.cbegin(), _buffer.cend()) << "$\n";
    if (!done())
    {
        Logger::getInstance() << "Body not fully read (Chunks are always read on one operation)\n"
        << "Read body size: " << _request.body.content.size() << ", "
        << "Content-Length: " << _request.headers.content_length << ", "
        << "Last Chunk size: " << _chunk_length << std::endl;
    }
}

std::string const  RequestParser::get_remainder() const
{
    return std::string(_buffer.cbegin(), _buffer.cend());
}

void RequestParser::append(uint8_t const * str, ssize_t size) {_buffer.append(str, size);}

void RequestParser::new_request()
{
    _empty_skip_count = 0;
    _status = PRS_FIRST_LINE;
    _buffer.new_request();
    _header_field_count = 0;
    _trailer_field_count = 0;
    _chunk_length = 0;
}

// void RequestParser::process()
// {
//     _processing = true;
//     while (_processing && _status != DONE && _error.status() == OK)
//     {
//         switch (_status)
//         {
//             case FIRST_LINE: parse_first_line(); break ;
//             case HEADERS: parse_header_line(); break ;
//             case BODY: parse_body() ; break ;
//             case CHUNKED_SIZE: parse_chunked_size(); break ;
//             case CHUNKED_BODY: parse_chunked_body(); break ;
//             case TRAILERS: parse_trailer_line(); break ;
//             case DONE: throw std::runtime_error("CODE ERROR: Must initialize a new request");
//         }
//     }
// }

RequestParser::parsing_status RequestParser::get_parsing_status() const {return _status;}

// Main parsing functions
bool RequestParser::test_first_line()
{
    _processing = _buffer.get_crlf_line(_begin, _end);
    if (_buffer.previous_read_size() >= RequestParser::FIRST_LINE_MAX_LENGTH)
    {
        _error.set("Request first line lgitength is too long. Max: " + RequestParser::FIRST_LINE_MAX_LENGTH, BAD_REQUEST);
        return false;
    }
    if (!_processing)
        return false;
    if (_line.empty())
    {
        _empty_skip_count ++;
        if (_empty_skip_count > 1)
            _error.set("Only one empty line is allowed before request", BAD_REQUEST);
        return false;
     }
    parse::first_line_sanitize(_begin, _end);
    return true;
}


bool RequestParser::test_chunk_size()
{
    _processing = _buffer.get_crlf_line(_begin, _end);
    if (_buffer.previous_read_size() >= RequestParser::CHUNKED_SIZE_LINE_MAX_LENGTH)
    {
        _error.set("Chunk size line too long", BAD_REQUEST);
        return false;
    }
    return _processing;
}

bool RequestParser::test_chunk_body()
{
    return _buffer.get_chunk(_chunk_length, _begin, _end);
}

bool RequestParser::test_body()
{
    return _buffer.get_chunk(_request.headers.content_length, _begin, _end);
}

bool RequestParser::test_trailer_line()
{
    _processing = _buffer.get_crlf_line(_line);
    if (_buffer.previous_read_size() >= RequestParser::HEADER_LINE_MAX_LENGTH || _trailer_field_count >= RequestParser::MAX_TRAILER_FIELDS)
    {
        _error.set("Request trailer field", REQUEST_HEADER_FIELDS_TOO_LARGE);
        return false;
    }
    if (!_processing)
        _processing ;
    _trailer_field_count ++;
    if (_line.empty())
    {
        _status = PRS_DONE;
        return false;
    }
    return _processing;
}

bool RequestParser::test_header_line()
{
    _processing = _buffer.get_crlf_line(_begin, _end);
    if (_buffer.previous_read_size() >= RequestParser::HEADER_LINE_MAX_LENGTH || _header_field_count >= RequestParser::MAX_HEADER_FIELDS)
    {
        _error.set("Request headers", REQUEST_HEADER_FIELDS_TOO_LARGE);
        return false;
    }
    if (!_processing)
        _processing;
    _header_field_count ++;
    if (_line.empty())
    {
        if (!_request.headers.transfer_encodings.empty() && _request.headers.transfer_encodings.back().name == "chunked")
            _status = PRS_CHUNKED_SIZE;
        else
            _status = PRS_BODY;
        return false;
    }
    return _processing;
}

void RequestParser::parse_first_line()
{
    parse_method();
    parse_uri();
    parse_protocol();
}

void RequestParser::parse_header_line()
{
    std::string name, value;

    // Parse name
    // _token_start = _line.begin();
    // _token_end = wss::skip_until(_line.begin(), _line.end(), ":");
    // if (_token_end == _line.end())
    //     return _error_container.put_error("field name, ':' separator not found", _line, _token_end - 1, BAD_REQUEST);
    // if (_token_end == _token_start)
    //     _error_container.put_error("field name, empty", BAD_REQUEST);
    // _element_parser.parse_field_token(_token_start, _token_end, _line, name);
    
    // // Parse value
    // _token_start = wss::skip_ascii_whitespace(_token_end + 1, _line.end());
    // if (_token_start != _line.end())
    //     _token_end = wss::skip_ascii_whitespace_r(_line.end(), _token_start);
    // else
    //     _token_end = _line.end();
    // _element_parser.parse_field_value(_token_start, _token_end, _line, value);
    // if (name.empty())
    //     return ;

    // _request.headers.put(name, value);
}

void RequestParser::parse_body()
{
    _request.body.content = std::string(_begin, _end);
    _status = PRS_DONE;
}

void RequestParser::parse_chunked_size()
{
    // Delimit hex token
    _token_start = _line.begin();
    _token_end = wss::skip_hexa_token(_token_start, _line.end());
    if (_token_start == _token_end)
        return _error_container.put_error("chunk size line must have at least one digit", _line, _token_start, BAD_REQUEST);

    // Get hexa value
    _chunk_length = parse::s_to_hex(_token_start, _token_end, RequestParser::MAX_CHUNK_SIZE);
    if (_chunk_length == 0)
        _status = TRAILERS;
    else
        _status = CHUNKED_BODY;
    if (_chunk_length  >= RequestParser::MAX_CHUNK_SIZE)
        return _error_container.put_error("chunk size too big", BAD_REQUEST);
    if (_request.body.content.size() + _chunk_length >= RequestParser::MAX_CONTENT_LENGTH)
        return _error_container.put_error("chunked body too big", BAD_REQUEST);
}

void RequestParser::parse_chunked_body()
{
    _request.body.content += std::string(_begin, _end);
    _status = PRS_CHUNKED_SIZE;
}

void RequestParser::parse_trailer_line()
{
}



// Secondary parsing functions
void RequestParser::parse_uri()
{
    // Skip to start of URI
    if (_token_start != _line.end() && _token_start + 1 != _line.end() && *_token_start == ' ' && *(_token_start + 1) == ' ')
        _error_container.put_warning("URI, extra whitespace", _line, _token_start);
    _token_start = wss::skip_whitespace(_token_start, _line.end());
    if (_token_start == _line.end())
        return _error_container.put_error("URI not found", BAD_REQUEST);

    // Check URI length
    std::string::const_iterator uri_limit = wss::skip_until(_token_start, _token_end, " ");
    if (std::distance(_token_start, uri_limit) >= RequestParser::URI_MAX_LENGTH)
    {
        _token_start = uri_limit;
        return _error_container.put_error("Request uri", URI_TOO_LONG);
    }

    // Parse
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
    // Set default port if not found
    if (_request.uri.port == -1)
        _request.uri.port = 80;
}

void RequestParser::get_hier_part()
{ 
    _token_start += 2; // Skip '//'

    // Find and skip userinfo if present
    _token_end = wss::skip_until(_token_start, _line.end(), "@");
    if (_token_end != _line.end() && *_token_end == '@')
    {
        _error_container.put_error("authority, userinfo '@' is deprecated", _line, _token_end, BAD_REQUEST);
        _token_start = _token_end + 1;
    }
    if (_token_start == _line.end())
        return _error_container.put_error("host, not found", BAD_REQUEST);

    // Find host (Until path or end)
    _token_end = wss::skip_until(_token_start, _line.end(), " :?#/");
    if (_token_end == _token_start)
        _error_container.put_error("host, not found", _line, _token_end, BAD_REQUEST);
    else
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

// Header field parsing

void RequestParser::parse_host_field(std::string const & value)
{
    _token_start = value.begin();
    _token_end = wss::skip_until(value.begin(), value.end(), ":");
    _element_parser.parse_host(_token_start, _token_end, value, _request.headers.host);
    if (_token_end != value.end() && *_token_end == ':')
    {
        _token_start = _token_end + 1;
        _token_end = value.end();
        _element_parser.parse_port(_token_start, _token_end, value, _request.headers.port);
    }
    else
        _request.headers.port = 80;
}

void RequestParser::parse_content_length_field(std::string const & value)
{
    _token_start = value.begin();
    _token_end = value.end();
    if (value.find(",") == std::string::npos)
        _element_parser.parse_content_length_field(_token_start, _token_end, value, _request.headers.content_length);
    else 
    {
        std::vector<CommaSeparatedFieldValue> csfs;
        _element_parser.parse_comma_separated_values(_token_start, _token_end, value, csfs);
        for (std::vector<CommaSeparatedFieldValue>::const_iterator it = csfs.begin(); it != csfs.end(); it ++)
        {
            ssize_t prev_value = _request.headers.content_length;
            _token_start = it->name.begin();
            _token_end = it->name.end();

            if (!it->parameters.empty())
                return _error_container.put_error("Content-Length values can't have parameters", BAD_REQUEST);
            _element_parser.parse_content_length_field(_token_start, _token_end, it->name, _request.headers.content_length);
            if (prev_value != -1 && prev_value != _request.headers.content_length)
                return _error_container.put_error("Content-Length has incoherent, different values: " + value, BAD_REQUEST);
        }
    }
}

void RequestParser::parse_transfer_encoding_field(std::string const & value)
{
    _token_start = value.begin();
    _token_end = value.end();
    _element_parser.parse_comma_separated_values(_token_start, _token_end, value, _request.headers.transfer_encodings);
}

// Auxiliary

void RequestParser::process_headers()
{
    for (std::map<std::string, std::string>::iterator it = _request.headers.fields.begin(); it != _request.headers.fields.end(); it ++)
        for (wsHeaders const * hdr = headers; hdr->parser_f != NULL; hdr ++)
            if (it->first == hdr->name)
                (this->*hdr->parser_f)(it->second);
    _validator.validate_headers(_request, _request.headers);
}

bool RequestParser::has_authority() const
{
    return _token_start + 1 < _line.end() && *_token_start == '/' && *(_token_start + 1) == '/';
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
        return _error_container.put_error("URI schema, separator ':' not found", BAD_REQUEST);
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
    // Skip to start of Protocol and checks
    if (_token_start != _line.end() && _token_start + 1 != _line.end() && *_token_start == ' ' && *(_token_start + 1) == ' ')
        _error_container.put_warning("protocol, extra whitespace after uri", _line, _token_start);
    _token_start = wss::skip_whitespace(_token_start, _line.end());
    if (_token_start == _line.end())
        return _error_container.put_error("protocol not found", BAD_REQUEST);
    _token_end = wss::skip_until(_token_start, _line.end(), " ");

    // Parse
    _element_parser.parse_protocol(_token_start, _token_end, _line, _request.protocol);

    // Final checks
    if (_token_end != _line.end() && *_token_end == ' ')
        _error_container.put_warning("request line, extra whitespace after protocol", _line, _token_end);

    if (wss::skip_whitespace(_token_end, _line.end()) != _line.end())
        _error.set("Extra content after protocol", BAD_REQUEST);
} 
