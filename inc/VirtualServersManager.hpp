#ifndef VIRTUAL_SERVERS_MANAGER_HPP
#define VIRTUAL_SERVERS_MANAGER_HPP

#include "Server.hpp"
#include "HTTPRequest.hpp"
#include "RequestManager.hpp"
#include "HTTPError.hpp"
#include "ElementParser.hpp"
#include "SysBufferFactory.hpp"
#include "ResponseManager.hpp"
#include "Location.hpp"
#include "HTTPMethod.hpp"
#include "Status.hpp"
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <arpa/inet.h> 		// INET_ADDRSTRL
#include <stdexcept>
#include <unistd.h>			// close()
#include <sys/socket.h>		// accept(), socket functions
#include <netinet/in.h>		// sockaddr_in
#include <cstring>			// memset()
#include <sys/epoll.h>		// epoll


class VirtualServersManager {
public:
	struct ClientState {
		HTTPRequest 	request;
		HTTPError		error;
		ElementParser	element_parser;
		RequestManager request_manager;
		ResponseManager response_manager;
	
		ClientState(int client_fd)
			: element_parser(error)
			, request_manager(request, error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
			, response_manager(request, error, SysBufferFactory::SYSBUFF_SOCKET, client_fd)
			{}

		~ClientState() {
		}
		
		bool isRequestComplete() const {
			return request_manager.request_done();
		}

		bool hasError() const {
			return error.status() != OK;
		}
	
		static std::map<int, ClientState*> client_states;
		static ClientState* getOrCreateClientState(int client_fd);
		static void cleanupClientState(int client_fd);
	};

private:
	std::vector<Server>             _servers;
	int                             _epoll_fd;
	std::vector<struct epoll_event> _events;
	std::vector<int>                _client_fds;

	void setupEpoll();
	bool isServerFD(int fd) const;
	int findServerIndex(int fd) const;
	void disconnectClient(int client_fd);
	void handleEvent(const struct epoll_event& event);
	void handleClientData(int client_fd);
	void handleNewConnection(int server_index);
	
	// metodos de request
	void processCompleteRequest(int client_fd, HTTPRequest& request);
	Server* findServerForRequest(const HTTPRequest& request);
	bool isMethodAllowed(Location* location, HTTPMethod method);
	
	// proceso y aux de cgi
	bool isCgiRequest(Location* location, const std::string& path);
	void processCgiRequest(int client_fd, HTTPRequest& request, Location* location);
	void processStaticRequest(int client_fd, Location* location);
	
	void sendErrorResponse(int client_fd, int status_code, const std::string& message);
	void sendErrorResponse(int client_fd, const HTTPError& error);

	void temp_sendSimpleResponse(int client_fd);

public:
	VirtualServersManager();
	VirtualServersManager(const ParsedServers& configs);
	~VirtualServersManager();
	void run();
};

#endif