#ifndef VIRTUAL_SERVERS_MANAGER_HPP
#define VIRTUAL_SERVERS_MANAGER_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sys/socket.h> // socket(), bind(), listen(), setsockopt()
#include <netinet/in.h> // sockaddr_in, INADDR_ANY...
#include <arpa/inet.h> // inet_addr(), inet_pton()
#include <fcntl.h> // fcntl()
#include <cstring> // memset()
#include <netdb.h> // getaddrinfo(), addrinfo
#include <unistd.h> // close()
#include <cerrno> // errno
#include <stdexcept>
#include <sys/epoll.h> // epoll_create1(), epoll_ctl(), epoll_wait()

#include "Listen.hpp"
#include "Parsed.hpp"
#include "Servers.hpp"
#include "HttpRequest.hpp"

class VirtualServersManager {
public:
    typedef std::vector<VirtualServerInfo> VirtualServerGroup;
    typedef std::map<VirtualServerKey, VirtualServerGroup> VirtualServersMap;
    typedef std::map<VirtualServerKey, int> ServersToSocketsMap;
    
    VirtualServersMap serversManager;
    ServersToSocketsMap serversToSockets;

    VirtualServersManager();
    ~VirtualServersManager();

    void addServer(VirtualServerInfo& server);
    bool loadFromParsedConfig(const ParsedServers& ps);
    bool initializeSockets(void);
	bool bindSockets(void);
    bool socketsListen(void);
    void cleanupSockets();
};

#endif