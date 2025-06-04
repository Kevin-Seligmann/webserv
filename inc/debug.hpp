#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "ServersManager.hpp"
#include "Parsed.hpp"
#include "Utils.hpp"

// DEBUG FUNCTIONS

void showParse(const ParsedServers& config);

void showServers(const ServersManager& ws);

#endif