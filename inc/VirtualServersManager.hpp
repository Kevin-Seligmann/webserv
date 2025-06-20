#ifndef VIRTUAL_SERVERS_MANAGER_HPP
#define VIRTUAL_SERVERS_MANAGER_HPP

#include <arpa/inet.h> // inet_addr(), inet_pton()
#include <cerrno> // errno
#include <cstring> // memset()
#include <fcntl.h> // fcntl()
#include <iostream>
#include <map>
#include <netdb.h> // getaddrinfo(), addrinfo
#include <netinet/in.h> // sockaddr_in, INADDR_ANY...
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h> // socket(), bind(), listen(), setsockopt()
#include <unistd.h> // close()
#include <vector>

#include "Servers.hpp"

class VirtualServersManager {
public:
    typedef std::vector<Servers::VirtualServerInfo> VirtualServerGroup;
    typedef std::map<Listen::VirtualServerKey, VirtualServerGroup> VirtualServersMap;
    typedef std::map<Listen::VirtualServerKey, int> ServersToSocketsMap;
    private:
    
private:
    VirtualServersMap serversManager;
    ServersToSocketsMap serversToSockets;
    
public:
    VirtualServersManager();
    ~VirtualServersManager();

    // initialize servers
    void addServer(Servers::VirtualServerInfo& server);
    bool loadFromParsedConfig(const ParsedServers& ps);
    bool initializeSockets(void);
	bool bindSockets(void);
    bool socketsListen(void);
    void cleanupSockets();

    // start servers
    bool createEpoll();
    
    // server lookup
    Servers::VirtualServerInfo* getServerForKey(const Listen::VirtualServerKey& key);
    
    // socket access
    ServersToSocketsMap& getServersToSockets() { return serversToSockets; }
    const ServersToSocketsMap& getServersToSockets() const { return serversToSockets; }
    const VirtualServersMap& getServersManager() const { return serversManager; }
};

#endif