#include "StreamRequest.hpp"

StreamRequest::StreamRequest(){reset();}

void StreamRequest::set_request_read_fd(int fd){_request_body_buffer.set_read_end(fd);}
void StreamRequest::set_cgi_write_fd(int fd){_request_body_buffer.set_write_end(fd);}
void StreamRequest::set_cgi_read_fd(int fd){_cgi_response_buffer.set_read_end(fd);}
void StreamRequest::set_response_write_fd(int fd){_cgi_response_buffer.set_write_end(fd);}
void StreamRequest::reset(){
    _request_body_buffer.reset(); 
    _cgi_response_buffer.reset();  
    request_body_size = 0; 
    cgi_response_body_size = 0;
    request_body_size_consumed = 0;
    cgi_response_body_size_consumed = 0;
    request_read_finished = false;
    request_write_finished = false;
    cgi_read_finished = false;
    cgi_write_finished = false;
    streaming_active = false;
    request_body_size_appended = 0;
}