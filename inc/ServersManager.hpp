#ifndef SERVERS_MANAGER
#define SERVERS_MANAGER

#include <string>
#include <vector>
#include <map>
#include "Parsed.hpp"
#include "Servers.hpp"
#include "Locations.hpp"
#include "HttpRequest.hpp"

struct HostPort {
    std::string host;
    int         port;

    HostPort(const std::string& host, const int port);
    ~HostPort();

    bool operator==(const HostPort& other) const;
    bool operator<(const HostPort& other) const;
};

struct ServersManager {
    std::map<HostPort, std::vector< Servers > > serversManager;

    ServersManager();
    ~ServersManager();

    void addServer(const HostPort& hostPort, Servers& server);
    // const Servers& matchRequestServer(const HttpRequest& req);
};


#endif