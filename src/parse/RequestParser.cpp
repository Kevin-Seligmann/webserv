#include "RequestParser.hpp"

const RequestParser::wsHeaders RequestParser::headers[] = {
    {"host", &RequestParser::parse_host_field},
    {"content-length", &RequestParser::parse_content_length_field},
    {"transfer-encoding", &RequestParser::parse_transfer_encoding_field},
    {"connection", &RequestParser::parse_connection_field},
    {"expect", &RequestParser::parse_expect_field},
    {"cookie", &RequestParser::parse_cookie_field},
    {"content-type", &RequestParser::parse_content_type_field},
    {"", NULL}
};

const size_t RequestParser::URI_MAX_LENGTH = 2000;
const size_t RequestParser::FIRST_LINE_MAX_LENGTH = 8000;
const size_t RequestParser::HEADER_LINE_MAX_LENGTH = 8000;
const size_t RequestParser::MAX_HEADER_FIELDS = 100;
const size_t RequestParser::MAX_TRAILER_FIELDS = 20;
const size_t RequestParser::MAX_CONTENT_LENGTH = 1024*1024*1024;
const size_t RequestParser::MAX_CHUNK_SIZE = 1024*1024*1024;;
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
    if (static_cast<size_t>(_buffer.previous_read_size()) >= RequestParser::FIRST_LINE_MAX_LENGTH)
    {
        _error.set("Request first line length is too long", BAD_REQUEST, true);
        return false;
    }
    if (!_processing)
        return false;
    if (_begin == _end)
    {
        _empty_skip_count ++;
        if (_empty_skip_count > 1)
            _error.set("Only one empty line is allowed before request", BAD_REQUEST, true);
        return false;
     }
    return true;
}

bool RequestParser::test_chunk_size()
{
    _processing = _buffer.get_crlf_line(_begin, _end);
    if (static_cast<size_t>(_buffer.previous_read_size()) >= RequestParser::CHUNKED_SIZE_LINE_MAX_LENGTH)
    {
        _error.set("Chunk size line too long", BAD_REQUEST, true);
        return false;
    }
    return _processing;
}

bool RequestParser::test_chunk_body()
{
    _processing = _buffer.get_crlf_chunk(_chunk_length, _begin, _end);
    if (_processing && (*(_end - 1) != '\n' || *(_end - 2) != '\r'))
    {
        _error.set("Expected new line at the end of chunked request", BAD_REQUEST, true);
        return false;
    }
    return _processing;
}

bool RequestParser::test_chunk_newline()
{
    _processing = _buffer.get_crlf_chunk(0, _begin, _end);
    if (_processing && (*(_end - 1) != '\n' || *(_end - 2) != '\r'))
    {
        std::ostringstream oss;
        oss << "Expected new line at the end of chunked request" ;
        _error.set(oss.str(), BAD_REQUEST, true);
        return false;
    }
    if (_processing)
        _status = PRS_CHUNKED_SIZE;
    return _processing;
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
    if (static_cast<size_t>(_buffer.previous_read_size()) >= RequestParser::HEADER_LINE_MAX_LENGTH || _trailer_field_count >= RequestParser::MAX_TRAILER_FIELDS)
    {
        _error.set("Request trailer field", REQUEST_HEADER_FIELDS_TOO_LARGE, true);
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
    if (static_cast<size_t>(_buffer.previous_read_size()) >= RequestParser::HEADER_LINE_MAX_LENGTH || _header_field_count >= RequestParser::MAX_HEADER_FIELDS)
    {
        _error.set("Request headers", REQUEST_HEADER_FIELDS_TOO_LARGE, true);
        return false;
    }
    if (!_processing)
        return _processing;
    _header_field_count ++;
    if (_begin == _end)
    {
        process_headers();
        if (_error.status() == OK)
        {
            if (!_request.headers.transfer_encodings.empty() && _request.headers.transfer_encodings.back().name == "chunked")
                _status = PRS_CHUNKED_SIZE;
            else
                _status = PRS_BODY;
            return true;
        }
        return false;
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
        return _error.set("protocol not found", BAD_REQUEST, true);
    _end = wss::skip_until(_begin, _end, " ");

    // Parse
    _element_parser.parse_protocol(_begin, _end, _request.protocol);

    // Final checks
    if (_end != _end && *_end == ' ')
        Logger::getInstance().warning("request line, extra whitespace after protocol");

    if (wss::skip_whitespace(_end, _end) != _end)
        _error.set("Extra content after protocol", BAD_REQUEST, true);
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
        return _error.set("Token ':' not found on field", BAD_REQUEST, true);
    if (token_end == token_start)
        return _error.set("Field name is empty", BAD_REQUEST, true);
    _element_parser.parse_field_token(_begin, token_end, name);
    
    if (name == "*" || name == "close")
        return _error.set("Header field name " + name + " is reserved", BAD_REQUEST, true);
    // Parse value
    token_start = wss::skip_ascii_whitespace(token_end + 1, _end);
    if (token_start != _end)
        token_end = wss::skip_ascii_whitespace_r(_end, token_start);
    else
        token_end = _end;
    _element_parser.parse_field_value(token_start, token_end, value);

    _request.headers.put(name, value);

    // process_headers();
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
        return _error.set("The chunk size line must have at least one hex digit", BAD_REQUEST, true);

    // Get hexa value
    _chunk_length = parse::s_to_hex(token_start, token_end, RequestParser::MAX_CHUNK_SIZE);
    if (_chunk_length == 0)
        _status = PRS_TRAILER_LINE;
    else
        _status = PRS_CHUNKED_BODY;
    if (_chunk_length  >= RequestParser::MAX_CHUNK_SIZE)
        return _error.set("Chunk size too big", BAD_REQUEST, true);
    if (_request.body.content.size() + _chunk_length >= RequestParser::MAX_CONTENT_LENGTH)
        return _error.set("Chunked body too big", BAD_REQUEST, true);
}

void RequestParser::parse_chunked_body()
{
    _request.body.content += std::string(_begin, _end - 2);
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
        return _error.set("URI not found", BAD_REQUEST, true);

    // Check URI length
    std::string::iterator uri_limit = wss::skip_until(token_start, token_end, " ");
    if (static_cast<size_t>(std::distance(token_start, uri_limit)) >= RequestParser::URI_MAX_LENGTH)
        return _error.set("Request uri", URI_TOO_LONG, true);

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
        return _error.set("authority, userinfo '@' is deprecated", BAD_REQUEST, true);
    if (token_start == _end)
        return _error.set("Host not found on URI", BAD_REQUEST, true);

    // Find host (Until path or end)
    token_end = wss::skip_until(token_start, _end, " :?#/");
    if (token_end == token_start)
        return _error.set("Host not found on URI", BAD_REQUEST, true);
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
        std::vector<Coding> csfs;
        _element_parser.parse_comma_separated_values(token_start, token_end, csfs);
        for (std::vector<Coding>::iterator it = csfs.begin(); it != csfs.end(); it ++)
        {
            ssize_t prev_value = _request.headers.content_length;
            token_start = it->name.begin();
            token_end = it->name.end();

            if (!it->parameters.empty())
                return _error.set("Content-Length values can't have parameters", BAD_REQUEST, true);
            _element_parser.parse_content_length_field(token_start, token_end, _request.headers.content_length);
            if (prev_value != -1 && prev_value != _request.headers.content_length)
                return _error.set("Content-Length has incoherent, different values: " + value, BAD_REQUEST, true);
        }
    }
}

void RequestParser::parse_transfer_encoding_field(std::string & value)
{
    parse_list(value.begin(), value.end(), &RequestParser::parse_transfer_encoding_element);
}

void RequestParser::parse_connection_field(std::string & value)
{
    parse_list(value.begin(), value.end(), &RequestParser::parse_connection_element);
}

void RequestParser::parse_expect_field(std::string & value)
{
    parse_list(value.begin(), value.end(), &RequestParser::parse_expect_element);
}

/*
    parameters      = *( OWS ";" OWS [ parameter ] )
    parameter       = parameter-name "=" parameter-value
    parameter-name  = token
    parameter-value = ( token / quoted-string )

    1. See if parameter exists
    2. Skip OWS;OWS - If not error
    3. Parse token=
    4. Parse token/quotedstring
*/
void RequestParser::parse_parameters(std::string::iterator begin, std::string::iterator end, std::vector<std::pair<std::string, std::string> > & parameters)
{
    std::string::iterator value_end, value_start, name_end, name_start, head;

    while (begin != end && _error.status() == OK) // 1
    {
        // 2
        head = wss::skip_ascii_whitespace(begin, end);
        if (head == end || *head != ';')
            return _error.set("Parameters, expected ; separator", BAD_REQUEST, true);
        head = wss::skip_ascii_whitespace(head + 1, end);

        // 3
        name_start = head;
        name_end = wss::skip_http_token(name_start, end);
        if (name_end == name_start || name_end == end || *name_end != '=')
            return _error.set("Parameters, name must be non empty and precede a '='", BAD_REQUEST, true);
        
        // 4
        head = name_end + 1;
        if (head == end)
            return _error.set("Parameters, value not found", BAD_REQUEST, true);
        if (head != end && *head == '"')
        {
            std::string val;
            std::string::iterator dquote_end = wss::skip_until_dquoted_string_end(head + 1, end);
            if (dquote_end == end || *dquote_end != '"')
                return _error.set("Parameters, closing dquote missing", BAD_REQUEST, true);
            _element_parser.parse_dquote_string(head, dquote_end, val); // Head and end points to ", updates head to last " 
            parameters.push_back(std::make_pair(std::string(name_start, name_end), val));
            head = dquote_end + 1;
        }
        else 
        {
            value_start = head;
            value_end = wss::skip_http_token(value_start, end);
            parameters.push_back(std::make_pair(std::string(name_start, name_end), std::string(value_start, value_end)));
            head = value_end;
        }
        begin = head;
    }
}

void RequestParser::parse_content_type_field(std::string & value)
{
    std::string::iterator head = value.begin();

    while (head != value.end() && parse::is_token_char(*head))
        head ++;
    _request.headers.content_type.type = std::string(value.begin(), head);

    if (head == value.end() || *head != '/')
        return _error.set("Media-type, expected subtype separator '/'", BAD_REQUEST, true);
    head ++;

    std::string::iterator begin = head;
    while (head != value.end() && parse::is_token_char(*head))
        head ++;
    _request.headers.content_type.subtype = std::string(begin, head);
    
    wss::to_lower(_request.headers.content_type.type);
    wss::to_lower(_request.headers.content_type.subtype);

    if (_request.headers.content_type.type == "" || _request.headers.content_type.subtype == "")
        return _error.set("Media-Type, empty type or subtype", BAD_REQUEST, true);
    
    parse_parameters(head, value.end(), _request.headers.content_type.parameters);
}

void RequestParser::parse_cookie_field(std::string & value)
{
    std::string::iterator head = value.begin();
    std::string::iterator name_start, name_end, value_start, value_end;
    bool dquote;

    while (head != value.end())
    {
        // Reset tokens
        name_start = head;
        name_end = head;

        // Parse name
        while (name_end != value.end() && parse::is_token_char(*name_end))
            name_end ++;
        if (name_start == name_end)
            return _error.set("Cookie name, empty", BAD_REQUEST, true);
        if (name_end == value.end() || *name_end != '=')
            return _error.set("Cookie name, '=' not found/unexpected character", BAD_REQUEST, true);
       
        // Parse value
        value_start = name_end + 1;
        if (value_start != value.end() && *value_start == '"')
        {
            value_start ++;
            dquote = true;
        }
        else
            dquote = false;

        head = value_start;
        while (head != value.end() && parse::is_cookie_char(*head))
            head ++;
        if (dquote && (head == value.end() || *head != '"'))
            return _error.set("Cookie value, unclosed quote/unexpected character", BAD_REQUEST, true);
        value_end = head;
        _request.headers.put_cookie(name_start, name_end, value_start, value_end);
        if (dquote)
            head ++;
    
        // Skip (";" SP). If any found, it's a different cookie. Else, last.
        if (head == value.end())
            return ;
        if (*head != ';')
            return _error.set("Cookie, unexpected character", BAD_REQUEST, true);
        head ++;
        if (head == value.end() || *head != ' ')
            return _error.set("Cookie, unexpected character", BAD_REQUEST, true);
        head ++;
    }
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
    (void)token_end; // Suppress unused parameter warning
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


void parse_dquote_string()
{

}

/*
    #element => [ element ] *( OWS "," OWS [ element ] )

    LOOP:
    1. Check if element - If, parse.
    2. If end, return
    2. Skip WS
    3. "," or error
    4. Skip WS

    element_parser should return an iterator after it last element or put an error.
*/
void RequestParser::parse_list (
    std::string::iterator token_start,    
    std::string::iterator token_end, 
    std::string::iterator (RequestParser::*element_parser)(std::string::iterator token_start, std::string::iterator token_end))
{
    std::string::iterator head = token_start;
    while (head != token_end)
    {
        if (*head != ' ' && *head != ',' && *head != '\t')
            head = (this->*element_parser)(token_start, token_end);
        if (head == token_end || _error.status() != OK)
            return ;
        
        head = wss::skip_whitespace(head, token_end);
        if (head == token_end || *head != ',')
            return _error.set("Parsing list, unexpected character", BAD_REQUEST, true);
        head ++;
        head = wss::skip_whitespace(head, token_end);
        token_start = head;
    }
}

std::string::iterator RequestParser::parse_transfer_encoding_element(std::string::iterator begin, std::string::iterator end)
{
    Coding csf;
    std::string::iterator head;

    // Get name
    head = wss::skip_until(begin, end, "; ");
    _element_parser.parse_field_token(begin, head, csf.name);
    if (csf.name.empty())
    {
        _error.set("Transfer encoding field, empty field name", BAD_REQUEST, true);
        return end;
    }
    
    // Parse params
    begin = wss::skip_ascii_whitespace(head, end);
    while (begin != end && *begin == ';')
    {
        std::string param_name, param_value;
    
        // Param name
        begin = wss::skip_ascii_whitespace(begin + 1, end);
        head = wss::skip_until(begin, end, "= \t");
        _element_parser.parse_field_token(begin, head, param_name);
        if (param_name.empty())
        {
            _error.set("comma separated field, empty parameter name", BAD_REQUEST, true);
            return end;
        }
    
        // Parse value (Skip to value)
        begin = wss::skip_ascii_whitespace(head, end);
        if (begin != end && *begin != '=')
        {
            _error.set("Comma separated parameter, unexpected character", BAD_REQUEST, true);
            return end;
        }
        if (begin == end || begin + 1 == end)
        {
            _error.set("Comma separated parameter, empty parameter value", BAD_REQUEST, true);
            return end;
        }
        begin = wss::skip_ascii_whitespace(begin + 1, end);

        // Parse value
        if (*begin == '"')
        {
            head = wss::skip_until_dquoted_string_end(begin + 1, end);
            if (head == end)
            {
                _error.set("Comma separated parameter, closing dquote missing", BAD_REQUEST, true);
                return end;
            }
            _element_parser.parse_dquote_string(begin, head, param_value);

            head ++; // Skip the last '"'
        }
        else 
        {
            head = wss::skip_until(begin, end, ", \t");
            _element_parser.parse_dquote_string(begin, head, param_value);
        }
        if (param_value.empty())
        {
            _error.set("Comma separated parameter, empty parameter value", BAD_REQUEST, true);
            return end;
        }

        // Put value and push begin iterator
        csf.parameters.push_back(std::pair<std::string, std::string>(param_name, param_value));
        begin = wss::skip_ascii_whitespace(head, end);
    }
    _request.headers.transfer_encodings.push_back(csf);

    return head;
}

std::string::iterator RequestParser::parse_connection_element(std::string::iterator begin, std::string::iterator end)
{
    std::string::iterator head = begin;
    while (head != end && parse::is_token_char(*head))
        head ++;
    _request.headers.connections.push_back(std::string(begin, head));
    wss::to_lower(_request.headers.connections.back());
    return head;
}

std::string::iterator RequestParser::parse_expect_element(std::string::iterator begin, std::string::iterator end)
{
    std::string::iterator head = begin;
    while (head != end && parse::is_token_char(*head))
        head ++;
    _request.headers.expectations.push_back(std::string(begin, head));
    wss::to_lower(_request.headers.expectations.back());
    return head;
}


std::string const  RequestParser::get_remainder() const
{
    return std::string(_buffer.cbegin(), _buffer.cend());
}

ssize_t RequestParser::extract_buffer_chunk(uint8_t * dst, ssize_t buffer_size)
{    
    ssize_t extracted = std::min(_buffer.size(), buffer_size);
    memcpy(dst, _buffer.itbegin(), extracted);
    _buffer.unsafe_consume_bytes(extracted);
    return extracted;
}

size_t RequestParser::get_chunk_length()
{
    return _chunk_length;
}