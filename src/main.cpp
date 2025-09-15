#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include "Parsed.hpp"
#include "VirtualServersManager.hpp"
#include "DebugView.hpp"
#include "CGIInterpreter.hpp"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}


void testVirtualServersManager(const ParsedServers& configs) {
    printSeparator("TEST: VirtualServersManager");
    
    if (configs.empty()) {
        std::cout << "✗ No configurations to test" << std::endl;
        return;
    }
    
    try {
        VirtualServersManager manager(configs);
        
        std::cout << "✓ VirtualServersManager created" << std::endl;
        std::cout << "✓ Managing " << configs.size() << " servers" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error creating manager: " << e.what() << std::endl;
    }
}

void runInteractiveTest(const ParsedServers& configs) {
    printSeparator("INTERACTIVE TEST");
    
    if (configs.empty()) {
        std::cout << "✗ No configurations for interactive test" << std::endl;
        return;
    }
    
    std::cout << "Starting interactive server test..." << std::endl;
    std::cout << "This will start all configured servers" << std::endl;
    
    // Show what will be started
    for (size_t i = 0; i < configs.size(); ++i) {
        for (size_t j = 0; j < configs[i].listens.size(); ++j) {
            std::cout << "  Server on " << configs[i].listens[j].host 
                      << ":" << configs[i].listens[j].port;
            if (!configs[i].server_names.empty()) {
                std::cout << " (" << configs[i].server_names[0] << ")";
            }
            std::cout << std::endl;
        }
    }
    
    std::cout << "\nStarting servers..." << std::endl;
    
    try {
        VirtualServersManager manager(configs);
        
        std::cout << "Starting all servers..." << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;
        
        // Start the actual event loop
        manager.run();
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error running servers: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Webserv Test" << std::endl;
    std::cout << "============" << std::endl;
    
    // Flags & config filename preprocessing
    bool interactive = false;
    const char* configFilename = NULL;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--interactive") {
            interactive = true;
        } else if (!arg.empty() && arg[0] == '-') {
            // ignore unknown flags for now
        } else if (configFilename == NULL) {
            configFilename = argv[i];
        }
    }

    // Parse configuration
    ParsedServers parsedConfig;

    try {
        MediaType::load_types();
		CGIInterpreter::load_interpreters();

		std::cout << "HERE <<<<<<<" << std::endl;
		std::cout << CGIInterpreter::ACCEPTED_EXT << std::endl;

        // Prepare for parseProcess function
        int newArgc = (configFilename ? 2 : 1);
        char* newArgv[3];
        newArgv[0] = argv[0];
        if (configFilename) newArgv[1] = const_cast<char*>(configFilename);
        int result = parseProcess(newArgc, newArgv, parsedConfig);
        
        if (result != 0) {
            Logger::getInstance().error("Configuration parsing failed");
            return 1;
        }
        
        if (parsedConfig.empty()) {
            Logger::getInstance().error("✗ No server configurations found");
            return 1;
        }
        
        Logger::getInstance().info("✓ Configuration parsed successfully");
        
    } catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "✗ Error parsing configuration: " << e.what();
        Logger::getInstance().error(oss.str());
        return 1;
    }
    
    // DEBUG
    DebugView::printConfigSummary(parsedConfig);
        
    // Interactive test
    std::cout << "\nRun with config file for interactive testing" << std::endl;
    std::cout << "Example: ./webserver conf/test_simple.conf --interactive" << std::endl;

    if (interactive) {
        std::cout << "\nStarting servers interactively..." << std::endl;
        runInteractiveTest(parsedConfig);
        return 0; // if runInteractiveTest returns
    }

    return 0;
}
