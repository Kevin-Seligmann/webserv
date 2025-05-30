#ifndef SERVERS_MANAGER
#define SERVERS_MANAGER

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "HostPort.hpp"
#include "Parsed.hpp"
#include "Servers.hpp"
#include "Locations.hpp"
#include "HttpRequest.hpp"

struct ServersManager {
    std::map<HostPort, std::vector< Servers > > serversManager;

    ServersManager();
    ~ServersManager();

    void addServer(const HostPort& hostPort, Servers& server);
    // const Servers& matchRequestServer(const HttpRequest& req);
};


#endif