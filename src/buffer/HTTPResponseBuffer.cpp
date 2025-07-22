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

void HTTPResponseBuffer::put_header_time(std::string const & name, time_t time)
{
    char buffer[100];
    struct tm* gmtimeinfo = gmtime(&time); 
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtimeinfo);
    put_header(name, buffer);
}

void HTTPResponseBuffer::put_header_number(std::string const & name, size_t n)
{
    std::stringstream ss;
    ss << n;
    put_header(name, ss.str());
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

/*
    Reserves enough bytes to write from fd to buffer.
    Then, reads the fd into the buffer and returns the result.
*/
ssize_t HTTPResponseBuffer::write_from_fd(int fd, size_t n)
{
    reserve(n);

    ssize_t rb = read(fd, end(), n);
    if (rb > 0)
        consume_bytes(rb);
    return rb;
}

void HTTPResponseBuffer::put_new_line(){append("\r\n");}
