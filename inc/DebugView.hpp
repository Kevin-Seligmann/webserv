/*
#pragma once

#include <string>
#include <vector>
#include "Parsed.hpp"
#include "Logger.hpp"

class VirtualServersManager;
*/

/*
class DebugView {
public:
    static void printConfigSummary(const ParsedServers &configs);
    static void printBox(const std::string &title, const std::vector<std::string> &lines);
};
*/

#ifndef DEBUG_HPP
#define DEBUG_HPP

#ifdef DEBUG_MODE
    #define DEBUG_LOG(x) std::cerr << "[DEBUG] " << x << std::endl;
    #define DEBUG_VAR(var) std::cerr<< "[VAR] " << var << std::endl;
#else
    #define DEBUG_LOG(x) do {} while(0)
    #define DEBUG_VAR(var) do {} while(0)
#endif

#endif