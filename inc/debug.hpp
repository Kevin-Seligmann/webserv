#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "VirtualServersManager.hpp"
#include "Parsed.hpp"
#include "Utils.hpp"
#include "Server.hpp"

// DEBUG FUNCTIONS

void showParse(const ParsedServers& config);

void showServers(const VirtualServersManager& ws);

void showSockets(const VirtualServersManager& ws);

#endif