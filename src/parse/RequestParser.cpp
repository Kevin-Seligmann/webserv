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

RequestParser::RequestParser(HTTPRequest & request, HTTPError & error, ElementParser & element_parser)
:_request(request), 
_error(error),
_element_parser(element_parser),
_status(PRS_FIRST_LINE),
_empty_skip_count(0),
_header_field_count(0),
_trailer_field_count(0),
_chunk_length(0)
{}

// Request parse managment
void RequestParser::new_request()
{
    _empty_skip_count = 0;
    _status = PRS_FIRST_LINE;
    _buffer.new_request();
    _header_field_count = 0;
    _trailer_field_count = 0;
    _chunk_length = 0;
}

RequestParser::parsing_status RequestParser::get_status() const {return _status;}

void RequestParser::append(uint8_t const * str, ssize_t size) {_buffer.append(str, size);}


// Parsing - Testers
bool RequestParser::test_first_line()
{
    _processing = _buffer.get_crlf_line(_begin, _end);
    if (_buffer.previous_read_size() >= RequestParser::FIRST_LINE_MAX_LENGTH)
    {
        _error.set("Request first line length is too long", BAD_REQUEST);
        return false;
    }
    if (!_processing)
        return false;
    if (_begin == _end)
    {
        _empty_skip_count ++;
        if (_empty_skip_count > 1)
            _error.set("Only one empty line is allowed before request", BAD_REQUEST);
        return false;
     }
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
    if (_request.headers.content_length <= 0)
    {
        _status = PRS_DONE;
        return false;
    }
    return _buffer.get_chunk(_request.headers.content_length, _begin, _end);
}

bool RequestParser::test_trailer_line()
{
    _processing = _buffer.get_crlf_line(_begin, _end);
    if (_buffer.previous_read_size() >= RequestParser::HEADER_LINE_MAX_LENGTH)
    {
        _error.set("Request trailer field line length", REQUEST_HEADER_FIELDS_TOO_LARGE);
        return false;
    }
    if (_trailer_field_count >= RequestParser::MAX_TRAILER_FIELDS)
    {
        _error.set("Request trailer field quantity", REQUEST_HEADER_FIELDS_TOO_LARGE);
        return false;
    }
    if (!_processing)
        return _processing ;
    _trailer_field_count ++;
    if (_begin == _end)
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
        return _processing;
    _header_field_count ++;
    if (_begin == _end)
    {
        process_headers();
        if (_error.status() != OK)
            return false;
        if (!_request.headers.transfer_encodings.empty() && _request.headers.transfer_encodings.back().name == "chunked")
            _status = PRS_CHUNKED_SIZE;
        else
            _status = PRS_BODY;
        return true;
    }
    return _processing;
}

void RequestParser::parse_first_line()
{
    parse::first_line_sanitize(_begin, _end);
    parse_method();
    parse_uri();
    parse_protocol();
    _status = PRS_HEADER_LINE;
}

void RequestParser::parse_method()
{
    if (*_begin == ' ')
    {
        Logger::getInstance().warning("First line, space before method");
        _begin = wss::skip_whitespace(_begin, _end);
    }

    std::string::iterator token_end = wss::skip_until(_begin, _end, " ");
    _element_parser.parse_method(_begin, token_end, _request.method);
    _begin = token_end;
}

void RequestParser::parse_protocol()
{
    // Skip to start of Protocol and checks
    if (_begin != _end && _begin + 1 != _end && *_begin == ' ' && *(_begin + 1) == ' ')
        Logger::getInstance().warning("protocol, extra whitespace after uri");
    _begin = wss::skip_whitespace(_begin, _end);
    if (_begin == _end)
        return _error.set("protocol not found", BAD_REQUEST);
    _end = wss::skip_until(_begin, _end, " ");

    // Parse
    _element_parser.parse_protocol(_begin, _end, _request.protocol);

    // Final checks
    if (_end != _end && *_end == ' ')
        Logger::getInstance().warning("request line, extra whitespace after protocol");

    if (wss::skip_whitespace(_end, _end) != _end)
        _error.set("Extra content after protocol", BAD_REQUEST);
} 

void RequestParser::parse_header_line()
{
    if (_begin == _end)
        return ;
    
    std::string name, value;
    std::string::iterator token_end, token_start = _begin;
    
    // Parse name
    token_end = wss::skip_until(token_start, _end, ":");
    if (token_end == _end)
        return _error.set("Token ':' not found on field", BAD_REQUEST);
    if (token_end == token_start)
        return _error.set("Field name is empty", BAD_REQUEST);
    _element_parser.parse_field_token(_begin, token_end, name);
    
    // Parse value
    token_start = wss::skip_ascii_whitespace(token_end + 1, _end);
    if (token_start != _end)
        token_end = wss::skip_ascii_whitespace_r(_end, token_start);
    else
        token_end = _end;
    _element_parser.parse_field_value(token_start, token_end, value);

    _request.headers.put(name, value);
}

void RequestParser::parse_body()
{
    _request.body.content = std::string(_begin, _end);
    _status = PRS_DONE;
}

void RequestParser::parse_chunked_size()
{
    std::string::iterator token_end, token_start = _begin;

    // Delimit hex token
    token_end = wss::skip_hexa_token(token_start, _end);
    if (token_start == token_end)
        return _error.set("The chunk size line must have at least one hex digit", BAD_REQUEST);

    // Get hexa value
    _chunk_length = parse::s_to_hex(token_start, token_end, RequestParser::MAX_CHUNK_SIZE);
    if (_chunk_length == 0)
        _status = PRS_TRAILER_LINE;
    else
        _status = PRS_CHUNKED_BODY;
    if (_chunk_length  >= RequestParser::MAX_CHUNK_SIZE)
        return _error.set("Chunk size too big", BAD_REQUEST);
    if (_request.body.content.size() + _chunk_length >= RequestParser::MAX_CONTENT_LENGTH)
        return _error.set("Chunked body too big", BAD_REQUEST);
}

void RequestParser::parse_chunked_body()
{
    _request.body.content += std::string(_begin, _end);
    _status = PRS_CHUNKED_SIZE;
}

void RequestParser::parse_uri()
{
    std::string::iterator token_end, token_start = _begin;

    // Skip to start of URI
    if (token_start != _end && token_start + 1 != _end && *token_start == ' ' && *(token_start + 1) == ' ')
        Logger::getInstance().warning("Extra white space after request URI (Client #)");
    token_start = wss::skip_whitespace(token_start, _end);
    if (token_start == _end)
        return _error.set("URI not found", BAD_REQUEST);

    // Check URI length
    std::string::iterator uri_limit = wss::skip_until(token_start, token_end, " ");
    if (std::distance(token_start, uri_limit) >= RequestParser::URI_MAX_LENGTH)
        return _error.set("Request uri", URI_TOO_LONG);

    // Parse
    if (token_start != _end && *token_start == '/')
        get_path(token_start, token_end);
    else if (token_start != _end)
    {
        get_schema(token_start, token_end);
        if (has_authority(token_start, token_end))
            get_hier_part(token_start, token_end);
        else
            get_path(token_start, token_end);
    }
    if (token_start != _end && *token_start == '?')
        get_query(token_start, token_end);
    if (token_start != _end && *token_start == '#')
        get_fragment(token_start, token_end);

    // Set default port if not found
    if (_request.uri.port == -1)
        _request.uri.port = 80;

    _begin = token_start;
}

// Secondary parsing functions
void RequestParser::get_hier_part(std::string::iterator & token_start, std::string::iterator & token_end)
{ 
    token_start += 2; // Skip '//'

    // Find and skip userinfo if present
    token_end = wss::skip_until(token_start, _end, "@");
    if (token_end != _end && *token_end == '@')
        return _error.set("authority, userinfo '@' is deprecated", BAD_REQUEST);
    if (token_start == _end)
        return _error.set("Host not found on URI", BAD_REQUEST);

    // Find host (Until path or end)
    token_end = wss::skip_until(token_start, _end, " :?#/");
    if (token_end == token_start)
        return _error.set("Host not found on URI", BAD_REQUEST);
    else
        _element_parser.parse_host(token_start, token_end, _request.uri.host);
    token_start = token_end;

    // Find port
    if (token_end != _end && *token_end == ':')
    {
        token_start ++;
        token_end = wss::skip_until(token_start, _end, " ?#/");
        _element_parser.parse_port(token_start, token_end, _request.uri.port);
        token_start = token_end;
    }

    if (token_end != _end && *token_end == '/')
        get_path(token_start, token_end);
}

// Header field parsing

void RequestParser::parse_host_field(std::string & value)
{
    std::string::iterator token_start = value.begin();
    std::string::iterator token_end = wss::skip_until(value.begin(), value.end(), ":");
    _element_parser.parse_host(token_start, token_end, _request.headers.host);
    if (token_end != value.end() && *token_end == ':')
    {
        token_start = token_end + 1;
        token_end = value.end();
        _element_parser.parse_port(token_start, token_end, _request.headers.port);
    }
    else
        _request.headers.port = 80;
}

void RequestParser::parse_content_length_field(std::string & value)
{
    std::string::iterator token_start = value.begin();
    std::string::iterator token_end = value.end();

    if (value.find(",") == std::string::npos)
        _element_parser.parse_content_length_field(token_start, token_end, _request.headers.content_length);
    else 
    {
        std::vector<CommaSeparatedFieldValue> csfs;
        _element_parser.parse_comma_separated_values(token_start, token_end, csfs);
        for (std::vector<CommaSeparatedFieldValue>::iterator it = csfs.begin(); it != csfs.end(); it ++)
        {
            ssize_t prev_value = _request.headers.content_length;
            token_start = it->name.begin();
            token_end = it->name.end();

            if (!it->parameters.empty())
                return _error.set("Content-Length values can't have parameters", BAD_REQUEST);
            _element_parser.parse_content_length_field(token_start, token_end, _request.headers.content_length);
            if (prev_value != -1 && prev_value != _request.headers.content_length)
                return _error.set("Content-Length has incoherent, different values: " + value, BAD_REQUEST);
        }
    }
}

void RequestParser::parse_transfer_encoding_field(std::string & value)
{
    std::string::iterator token_start = value.begin();
    std::string::iterator token_end = value.end();
    _element_parser.parse_comma_separated_values(token_start, token_end, _request.headers.transfer_encodings);
}

// Auxiliary

void RequestParser::process_headers()
{
    for (std::map<std::string, std::string>::iterator it = _request.headers.fields.begin(); it != _request.headers.fields.end(); it ++)
        for (wsHeaders const * hdr = headers; hdr->parser_f != NULL; hdr ++)
            if (it->first == hdr->name)
                (this->*hdr->parser_f)(it->second);
}

bool RequestParser::has_authority(std::string::iterator & token_start, std::string::iterator & token_end) const
{
    return token_start + 1 < _end && *token_start == '/' && *(token_start + 1) == '/';
}

void RequestParser::get_path(std::string::iterator & token_start, std::string::iterator & token_end)
{
    token_end = wss::skip_until(token_start, _end, " ?#");
    _element_parser.parse_path(token_start, token_end, _request.uri.path);
    token_start = token_end;
}


void RequestParser::get_schema(std::string::iterator & token_start, std::string::iterator & token_end)
{
    token_end = wss::skip_until(token_start, _end, ":");
    if (token_end == _end)
        return _error.set("URI schema, separator ':' not found", BAD_REQUEST);
    _element_parser.parse_schema(token_start, token_end, _request.uri.schema);
    token_start = token_end + 1;
}

void RequestParser::get_query(std::string::iterator & token_start, std::string::iterator & token_end)
{
    token_start ++;
    token_end = wss::skip_until(token_start, _end, " #");
    _element_parser.parse_query(token_start, token_end, _request.uri.query);
    token_start = token_end;
}

void RequestParser::get_fragment(std::string::iterator & token_start, std::string::iterator & token_end)
{
    token_start ++;
    token_end = wss::skip_until(token_start, _end, " ");
    _element_parser.parse_fragment(token_start, token_end, _request.uri.fragment);
    token_start = token_end;
}

// Debug
void RequestParser::dump_remainder() const
{
    Logger::getInstance() << "Buffer remainder:\n" << std::string(_buffer.cbegin(), _buffer.cend()) << "$\n";
    if (_status != PRS_DONE)
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