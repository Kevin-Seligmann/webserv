#pragma once

#include "HTTPRequest.hpp"
#include "HTTPError.hpp"
#include "ElementParser.hpp"
#include "RequestManager.hpp"
#include "ResponseManager.hpp"

class VirtualServersManager;

class Client 
{
public:
    Client(VirtualServersManager & vsm, int client_fd);

    ~Client();

    void process(int fd, int mode);
    int  getSocket() const;
    int  ownsFd(int fd) const;

private:
    enum Status {
        PROCESSING_REQUEST,
        PROCESSING_RESPONSE,
        PROCESSING_CGI,
        CLOSING
    };

    static const int MAX_ERROR_RETRIES = 1;

    VirtualServersManager & _vsm;
    Status                  _status;
    HTTPError	            _error;
    HTTPRequest 	        _request;
    ElementParser	        _element_parser;
    RequestManager          _request_manager;
    ResponseManager         _response_manager;
    int                     _socket;
    id_t                    _error_retry_count;
    ActiveFileDescriptor    _active_fd;
    // int             error_retry_count;


    void handle_cgi_request();
    void handle_processing_request();
    void handle_processing_response();
    void handle_closing();

    void prepareResponse(ServerConfig * server, Location * location);
    void prepareRequest();

    bool isCgiRequest(Location* location, const std::string& path);
    void updateActiveFileDescriptor(ActiveFileDescriptor newfd);
    void updateActiveFileDescriptor(int fd, int mode);
    void changeStatus(Status new_status, const std::string& reason = "");
    std::string statusToString(Status status);
    void process_status_error();
	void get_config(ServerConfig ** server, Location ** location);

};