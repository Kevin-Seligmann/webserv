#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include "ServersManager.hpp"
#include "SocketsManager.hpp"
#include "Parsed.hpp"

// Server initialization functions
bool serversInit(ServersManager& sm, const ParsedServers& ps);
bool socketsInit(SocketsManager listenSockets, ServersManager webServers);

#endif