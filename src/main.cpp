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

    std::cout << "Webserv Test" << std::endl;
    std::cout << "============" << std::endl;
    
    const char* configFilename = NULL;

    if (argc > 2)
    {
        std::cerr << "✗ Wrong arguments number!" << std::endl;
        std::cerr << "Use: ./webserv <config.conf>" << std::endl;
        std::cerr << "Or use a default configuration using: ./webserv " << std::endl;
        return 1;
    }
    else if (argc == 2)
    {
        configFilename = argv[1];
    }

    // Parse configuration
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
        
	std::cout << "\nStarting servers interactively..." << std::endl;
    runServer(parsedConfig);
    return 0;
}
