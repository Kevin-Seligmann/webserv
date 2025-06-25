#include "HTTPResponseBuffer.hpp"

void HTTPResponseBuffer::put_protocol(std::string const & protocol) 
{
    append(protocol);
    append(" ");
}

void HTTPResponseBuffer::put_status(HTTPError const & err)
{
    append(status::stoa(err.status()));
    append(" ");
    append(err.msg());
    append("\r\n");
}

void HTTPResponseBuffer::put_header(std::string const & name, std::string const & value)
{
    append(name);
    append(": ");
    append(value);
    append("\r\n");
}

void HTTPResponseBuffer::put_body(std::string const & body)
{
    append(body);
}

void HTTPResponseBuffer::put_body_chunk(std::string const & chunk)
{
    append(wss::i_to_hex(chunk.size()));
    append("\r\n");
    append(chunk);
}

void HTTPResponseBuffer::put_new_line(){append("\r\n");}
