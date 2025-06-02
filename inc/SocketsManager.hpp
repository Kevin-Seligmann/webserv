#ifndef SOCKETSMANAGER_HPP
#define SOCKETSMANAGER_HPP

#include <map>
#include <string>
#include "HostPort.hpp"

struct SocketsManager {
    std::map<HostPort, int> listenSockets;

    SocketsManager();
    ~SocketsManager();

    bool isCreated(const HostPort& hp);
};

#endif