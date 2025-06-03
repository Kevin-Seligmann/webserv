#ifndef SERVERS_MANAGER
#define SERVERS_MANAGER

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "Listen.hpp"
#include "Parsed.hpp"
#include "Servers.hpp"
#include "HttpRequest.hpp"

struct ServersManager {
    std::map<Listen, std::vector< Servers > > serversManager;

    ServersManager();
    ~ServersManager();

    void addServer(Servers& server);
    // const Servers& matchRequestServer(const HttpRequest& req);
};


#endif