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

    static const int TIMEOUT_SECONDS = 30;
    static const int CLOSING_GRACE_PERIOD = 1;
    static const int KEEP_ALIVE_TIMEOUT = 5;

    Client(VirtualServersManager & vsm, int client_fd);

    ~Client();

    time_t getLastActivity() const { return _last_activity; }

    void process(int fd, int mode);

    int getSocket() const { return _socket; }
    int ownsFd(int fd) const { return fd == _socket || fd == _active_fd.fd; }
    bool closing() const;

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
    time_t                  _last_activity;
    // int             error_retry_count;


    void handle_cgi_request();
    void handle_processing_request();
    void handle_processing_response();
    void handle_closing();

    void handleRequestDone();
    void handleRequestError();

    void prepareResponse(ServerConfig * server, Location * location, ResponseManager::RM_error_action action);
    void prepareRequest();

    bool isCgiRequest(Location* location, const std::string& path);
    bool isKeepAlive() const;
    void updateActiveFileDescriptor(ActiveFileDescriptor newfd);
    void updateActiveFileDescriptor(int fd, int mode);

	void get_config(ServerConfig ** server, Location ** location);
};
