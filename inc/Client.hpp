#pragma once

#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

#include "HTTPRequest.hpp"
#include "HTTPError.hpp"
#include "ElementParser.hpp"
#include "RequestManager.hpp"
#include "ResponseManager.hpp"
#include "CGI.hpp"
#include "CGIInterpreter.hpp"

class VirtualServersManager;

class Client 
{
public:

    static const int TIMEOUT_SECONDS = 30000;
    static const int CLOSING_GRACE_PERIOD = 1;
    static const int KEEP_ALIVE_TIMEOUT = 5;

    Client(VirtualServersManager & vsm, int client_fd);

    ~Client();

    time_t getLastActivity() const { return _last_activity; }

    void process(int fd);

    int getSocket() const { return _socket; }
    int ownsFd(int fd) const;
    bool closing() const;
    bool idle() const;


private:
    enum Status {
        IDLE,
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
    CGI                     _cgi;
    ElementParser	        _element_parser;
    RequestManager          _request_manager;
    ResponseManager         _response_manager;
    int                     _socket;
    id_t                    _error_retry_count;
    std::vector<ActiveFileDescriptor> _active_fds;
    time_t                  _last_activity;
    bool                    _is_cgi;
    std::string             _previous_directory_path; // Almacena ruta del último directorio

    void handle_cgi_request(int fd);
    void handle_processing_request();
    void handle_processing_response();

    void handleRequestDone();
    void handleRequestError();

    void prepareResponse(ServerConfig * server, Location * location, ResponseManager::RM_error_action action);
    void prepareRequest();
    void prepareCgi(ServerConfig* server, Location* location);

    bool isCgiRequest(Location* location);
    // Intento razonable de saber que el click viene de un directorio
    bool probablyAutoindex() const;

    void updateActiveFileDescriptor(ActiveFileDescriptor newfd);
    void updateActiveFileDescriptor(int fd, int mode);
    void updateActiveFileDescriptors(std::vector<ActiveFileDescriptor> fds);

	void get_config(ServerConfig ** server, Location ** location);
    void setStatus(Status status, std::string const & txt);
};
