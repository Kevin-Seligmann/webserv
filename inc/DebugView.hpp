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
#ifndef DEBUGVIEW_HPP
#define DEBUGVIEW_HPP

#ifndef DEBUG_MODE
    #define DEBUG_MODE 0
#endif
    
#if DEBUG_MODE == 1
    #include <iostream>
    #define DEBUG_LOG(msg) \
        do { std::cerr << "[DEBUG] " << msg << std::endl; } while(0)
    #define DEBUG_VAR(var) \
        do { std::cerr << "[DEBUG] " #var " = " << var << std::endl; } while(0)
#else
    #define DEBUG_LOG(msg) do {} while(0)
    #define DEBUG_VAR(var) do {} while(0)
#endif

#endif