#pragma once

#include "StreamBuffer.hpp"

class StreamRequest 
{


public:
    StreamRequest();

    void set_request_read_fd(int fd);
    void set_cgi_write_fd(int fd);
    void set_cgi_read_fd(int fd);
    void set_response_write_fd(int fd);

    void reset();

    bool streaming_active;
    size_t request_body_size;
    size_t cgi_response_body_size;
    size_t request_body_size_consumed;
    size_t cgi_response_body_size_consumed;
    size_t request_body_size_appended;
    bool request_read_finished;
    bool request_write_finished;
    bool cgi_read_finished;
    bool cgi_write_finished;

    StreamBuffer & get_request_buffer(){return _request_body_buffer;}
    StreamBuffer & get_response_buffer(){return _cgi_response_buffer;}

private:
    StreamBuffer _request_body_buffer;
    StreamBuffer _cgi_response_buffer;

};