#ifndef VIRTUAL_SERVERS_MANAGER_HPP
#define VIRTUAL_SERVERS_MANAGER_HPP

#include <vector>
#include <map>
#include <poll.h>        // Para pollfd
#include "Server.hpp"
#include "Parsed.hpp"    // Para ParsedServers
#include "HTTPRequest.hpp"

class VirtualServersManager {
private:
    std::vector<Server> _servers;
    std::vector<pollfd> _poll_fds;
    std::vector<int> _client_fds;
    std::map<int, HTTPRequest> _requests;

    void setupPollFDs();
    
public:
    VirtualServersManager();
    VirtualServersManager(const ParsedServers& configs);
    ~VirtualServersManager();

    void run();
};

#endif // VIRTUAL_SERVERS_MANAGER_HPP