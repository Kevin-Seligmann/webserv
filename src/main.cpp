#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include "Parsed.hpp"
#include "VirtualServersManager.hpp"
#include "DebugView.hpp"
#include "CGIInterpreter.hpp"

void runServer(const ParsedServers& configs)
{    
    if (configs.empty()) {
		Logger::getInstance().error("✗ No configurations for virtual servers");
        return;
    }
    
    Logger::getInstance().info(" === Starting Webserv ===");
    
    // Show servers config
    for (size_t i = 0; i < configs.size(); ++i)
	{
        for (size_t j = 0; j < configs[i].listens.size(); ++j)
		{
			std::ostringstream oss;
			oss << "Starting... "
				<< configs[i].listens[j].host
				<< ":" 
				<< configs[i].listens[j].port
				<< " for servername "
				<< (!configs[i].server_names.empty()
					? "'" + configs[i].server_names[0] + "'"
					: "Unnamed'");
			Logger::getInstance().info(oss.str());
        }
    }
    
    try
	{
        VirtualServersManager manager(configs);
        manager.run();
    }
	catch (const std::exception& e)
	{
		std::ostringstream oss;
		oss << "✗ Error running servers: "
			<< e.what();
		Logger::getInstance().error(oss.str());
    }
}

int main(int argc, char* argv[])
{

    DEBUG_LOG("\nDEBUG MODE ON\n");
    std::cout << "\n Webserv Test" << std::endl;
    std::cout << "==============\n" << std::endl;
    
    if (argc > 2)
    {
        Logger::getInstance().info("✗ Wrong arguments number!");
        Logger::getInstance().info("Use: ./webserv <config.conf>");
        Logger::getInstance().info("Or use a default configuration using: ./webserv ");
        return 1;
    }

    ParsedServers parsedConfig;

    try
    {
    
        MediaType::load_types();
        CGIInterpreter::load_interpreters();

        int result = parseProcess(argc, argv, parsedConfig);
        
        if (result != 0) {
            Logger::getInstance().error("✗ Configuration parsing failed");
            return 1;
        }
        
        if (parsedConfig.empty()) {
            Logger::getInstance().error("✗ No server configurations found");
            return 1;
        }
        
        Logger::getInstance().info("✓ Configuration parsed successfully");
        
    }
    catch (const std::exception& e)
    {
        std::ostringstream oss;
        oss << "✗ Error parsing configuration: " << e.what();
        Logger::getInstance().error(oss.str());
        return 1;
    }
    
    // DEBUG
    // DebugView::printConfigSummary(parsedConfig);
        
	Logger::getInstance().info("\nStarting servers interactively...");
    runServer(parsedConfig);
    return 0;
}

// TODO

/*
1. cuando un conf no tiene puntos y comas se rompe... estaba arreglado, pero sigue fallando
    esto puede ser porque lo hemos hecho mal con algun merge
2. alias resuelve mal, con alias = location, deberia quedarse solo con el alias

*/