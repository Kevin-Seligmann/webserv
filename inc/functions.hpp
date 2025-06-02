#ifndef SERVERS_INIT_HPP
#define SERVERS_INIT_HPP

#include "ServersManager.hpp"
#include "Parsed.hpp"

ParsedServers& fakeConfig(ParsedServers& ps);
bool serversInit(ServersManager& sm, const ParsedServers& ps);
bool socketsInit(SocketsManager listenSockets, ServersManager webServers);

#endif