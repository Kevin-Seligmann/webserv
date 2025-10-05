
#ifndef DEBUGVIEW_HPP
#define DEBUGVIEW_HPP

#include "Logger.hpp"

#ifndef DEBUG_MODE
    #define DEBUG_MODE 0
#endif
    
#if DEBUG_MODE == 1
    #include <iostream>
    #define DEBUG_LOG(msg) \
        do { Logger::getInstance() << msg << std::endl; } while(0)
#else
    #define DEBUG_LOG(msg) do {} while(0)
#endif

#endif