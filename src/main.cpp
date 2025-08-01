#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include "Server.hpp"
#include "Parsed.hpp"
#include "VirtualServersManager.hpp"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

void printParsedConfig(const ParsedServers& configs) {
    printSeparator("PARSED CONFIGURATION");
    
    std::cout << "Total servers: " << configs.size() << std::endl;
    
    for (size_t i = 0; i < configs.size(); ++i) {
        std::cout << "\n" << std::string(40, '-') << std::endl;
        std::cout << "SERVER " << (i + 1) << " CONFIGURATION" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        // =========================
        // NETWORK LAYER
        // =========================
        std::cout << "\n📡 NETWORK LAYER:" << std::endl;
        std::cout << "  Listen Directives:" << std::endl;
        for (size_t j = 0; j < configs[i].listens.size(); ++j) {
            std::cout << "    - " << configs[i].listens[j].host 
                      << ":" << configs[i].listens[j].port;
            if (configs[i].listens[j].is_default) {
                std::cout << " (default server)";
            }
            std::cout << std::endl;
        }
        
        // =========================
        // CONFIG LAYER
        // =========================
        std::cout << "\n⚙️  CONFIG LAYER:" << std::endl;
        
        // Server names
        if (!configs[i].server_names.empty()) {
            std::cout << "  Server Names: ";
            for (size_t j = 0; j < configs[i].server_names.size(); ++j) {
                std::cout << configs[i].server_names[j];
                if (j < configs[i].server_names.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
        } else {
            std::cout << "  Server Names: (none - accepts any host)" << std::endl;
        }
        
        // Basic configuration
        std::cout << "  Document Root: " << (configs[i].root.empty() ? "(not set)" : configs[i].root) << std::endl;
        std::cout << "  Directory Listing: " << (configs[i].autoindex ? "enabled" : "disabled") << std::endl;
        std::cout << "  Max Body Size: " << (configs[i].client_max_body_size.empty() ? "default" : configs[i].client_max_body_size) << std::endl;
        
        // Index files
        if (!configs[i].index_files.empty()) {
            std::cout << "  Index Files: ";
            for (size_t j = 0; j < configs[i].index_files.size(); ++j) {
                std::cout << configs[i].index_files[j];
                if (j < configs[i].index_files.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
        } else {
            std::cout << "  Index Files: (none configured)" << std::endl;
        }
        
        // Allowed methods
        if (!configs[i].allow_methods.empty()) {
            std::cout << "  Allowed Methods: ";
            for (size_t j = 0; j < configs[i].allow_methods.size(); ++j) {
                std::cout << configs[i].allow_methods[j];
                if (j < configs[i].allow_methods.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
        } else {
            std::cout << "  Allowed Methods: (all methods allowed)" << std::endl;
        }
        
        // Error pages
        if (!configs[i].error_pages.empty()) {
            std::cout << "  Error Pages:" << std::endl;
            for (std::map<int, std::string>::const_iterator it = configs[i].error_pages.begin();
                 it != configs[i].error_pages.end(); ++it) {
                std::cout << "    " << it->first << " -> " << it->second << std::endl;
            }
        } else {
            std::cout << "  Error Pages: (using default error pages)" << std::endl;
        }
        
        // =========================
        // LOCATION LAYER
        // =========================
        std::cout << "\n📂 LOCATION LAYER:" << std::endl;
        if (configs[i].locations.empty()) {
            std::cout << "  Locations: (no specific locations configured)" << std::endl;
        } else {
            std::cout << "  Configured Locations (" << configs[i].locations.size() << "):" << std::endl;
            for (std::map<std::string, Location>::const_iterator it = configs[i].locations.begin();
                 it != configs[i].locations.end(); ++it) {
                std::cout << "    📁 " << it->first << std::endl;
                // TODO: Add more location details when Location class has getters
            }
        }
    }
    
    std::cout << "\n" << std::string(40, '-') << std::endl;
    std::cout << "END OF CONFIGURATION" << std::endl;
    std::cout << std::string(40, '-') << std::endl;
}

void testServerCreation(const ParsedServers& configs) {
    printSeparator("TEST: Server Creation from Parsed Config");
    
    if (configs.empty()) {
        std::cout << "✗ No configurations to test" << std::endl;
        return;
    }
    
    try {
        // Test creating servers from parsed config
        for (size_t i = 0; i < configs.size(); ++i) {
            std::cout << "\n" << std::string(30, '-') << std::endl;
            std::cout << "🏗️  Creating Server " << (i + 1) << "..." << std::endl;
            std::cout << std::string(30, '-') << std::endl;
            
            Server server(configs[i]);
            
            std::cout << "✅ Server object created successfully" << std::endl;
            
            // Mostrar los miembros reales de la clase Server
            std::cout << std::endl;
            server.printNetworkLayer();
            std::cout << std::endl;
            server.printConfigLayer();
        }
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error creating server: " << e.what() << std::endl;
    }
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

void testServerBinding(const ParsedServers& configs) {
    printSeparator("TEST: Server Socket Binding");
    
    if (configs.empty()) {
        std::cout << "✗ No configurations to test" << std::endl;
        return;
    }
    
    try {
        // Test binding the first server only to avoid port conflicts
        std::cout << "🌐 Testing socket binding for first server..." << std::endl;
        std::cout << "   Target: " << configs[0].listens[0].host 
                  << ":" << configs[0].listens[0].port << std::endl;
        
        Server server(configs[0]);
        
        std::cout << "\n📡 NETWORK LAYER - After Construction (Before Binding):" << std::endl;
        server.printNetworkLayer();
        
        std::cout << "\n🚀 Starting server (binding socket)..." << std::endl;
        server.start();
        
        std::cout << "\n📡 NETWORK LAYER - After Socket Binding:" << std::endl;
        server.printNetworkLayer();
        
        std::cout << "\n⚙️  CONFIG LAYER - Current State:" << std::endl;
        server.printConfigLayer();
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error binding server: " << e.what() << std::endl;
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
    std::cout << "Webserv Test Suite (Using Parsed System)" << std::endl;
    std::cout << "=========================================" << std::endl;
    

    // Parse configuration
    ParsedServers parsedConfig;
    
    try {
        MediaType::load_types();

        // Use your existing parseProcess function
        int result = parseProcess(argc, argv, parsedConfig);
        
        if (result != 0) {
            std::cout << "✗ Configuration parsing failed" << std::endl;
            return 1;
        }
        
        if (parsedConfig.empty()) {
            std::cout << "✗ No server configurations found" << std::endl;
            return 1;
        }
        
        std::cout << "✓ Configuration parsed successfully" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error parsing configuration: " << e.what() << std::endl;
        return 1;
    }
    
    // Show parsed configuration
    printParsedConfig(parsedConfig);
    
    // Run tests
    testServerCreation(parsedConfig);
    testVirtualServersManager(parsedConfig);
    testServerBinding(parsedConfig);
    
    // Interactive test
    std::cout << "\nRun with config file for interactive testing" << std::endl;
    std::cout << "Example: ./webserv_test config/default.conf --interactive" << std::endl;
    
    // Check for interactive flag or run automatically if no flag
    bool interactive = true; // Default to interactive
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--test") {
            interactive = false;
            break;
        }
    }
    
    if (interactive) {
        std::cout << "\n🚀 Starting servers automatically..." << std::endl;
        runInteractiveTest(parsedConfig);
    }
    
    printSeparator("TESTS COMPLETED");
    std::cout << "All tests completed using parsed configuration!" << std::endl;
    std::cout << "Next steps:" << std::endl;
    std::cout << "1. Implement VirtualServersManager::run()" << std::endl;
    std::cout << "2. Add HTTP request parsing integration" << std::endl;
    std::cout << "3. Test with real config files" << std::endl;
    
    return 0;
}