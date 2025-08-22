#pragma once

#include <string>
#include <vector>
#include "Parsed.hpp"
#include "Logger.hpp"

class VirtualServersManager;

class DebugView {
public:
    static void printConfigSummary(const ParsedServers &configs);
    static void printBox(const std::string &title, const std::vector<std::string> &lines);
};


