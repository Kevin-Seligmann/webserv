#pragma once

#include "ServerConfig.hpp"
#include "HTTPRequest.hpp"
#include "HTTPError.hpp"
#include "ElementParser.hpp"
#include "RequestManager.hpp"
#include "ResponseManager.hpp"


class VirtualServersManager;

class Client 
{
public:
    Client(VirtualServersManager & vsm, std::vector<ServerConfig> & servers, int client_fd);

    ~Client();

    void process(int fd, int mode);
    int  getSocket() const;
    int  ownsFd(int fd) const;
    void changeStatus(Status new_status, const std::string& reason = "");

private:
    enum Status {
        PROCESING_REQUEST,
        PROCESING_RESPONSE,
        CLOSING,
        CLOSED
    };

    VirtualServersManager & _vsm;
	std::vector<ServerConfig> &   _servers;
    Status                  _status;
    HTTPError	            _error;
    HTTPRequest 	        _request;
    ElementParser	        _element_parser;
    RequestManager          _request_manager;
    ResponseManager         _response_manager;
    int                     _socket;
    ActiveFileDescriptor    _active_fd;
    
            
        private:

	ServerConfig* findServerForRequest(const HTTPRequest& request);
	Location* findLocationForRequest(const HTTPRequest& request, const ServerConfig* server);
    bool isCgiRequest(Location* location, const std::string& path);
    void processCgiRequest(Location* location);
    void request();
    void response();
    void prepareResponse(ServerConfig * server, Location * location);
    void prepareRequest();
    void updateActiveFileDescriptor(ActiveFileDescriptor newfd);
    void updateActiveFileDescriptor(int fd, int mode);
    std::string statusToString(Status status);

};