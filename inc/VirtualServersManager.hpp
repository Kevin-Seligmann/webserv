#ifndef VIRTUAL_SERVERS_MANAGER_HPP
#define VIRTUAL_SERVERS_MANAGER_HPP


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
#include "Client.hpp"

class VirtualServersManager {
private:
    std::map<int, Client *>		 	_clients;
	std::vector<Server>             _servers;
	int                             _epoll_fd;
	std::vector<struct epoll_event> _events;
	std::vector<int>                _client_fds;

	void setupEpoll();
	bool isServerFD(int fd) const;
	int findServerIndex(int fd) const;
	void disconnectClient(int client_fd);
	void handleEvent(const struct epoll_event& event);
	void handleNewConnection(int server_index);
	
    Client * searchClient(int client_fd);
    void cleanupClientState(int client_fd);

public:
	VirtualServersManager();
	VirtualServersManager(const ParsedServers& configs);
	~VirtualServersManager();

	void run();
	void hookFileDescriptor(ActiveFileDescriptor const & actf);
	void unhookFileDescriptor(ActiveFileDescriptor const & actf);
	void swapFileDescriptor(ActiveFileDescriptor const & oldfd, ActiveFileDescriptor const & newfd);
};

#endif