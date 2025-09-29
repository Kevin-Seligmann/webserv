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

3. en un listado de archivos, la direccion explicita se lista, pero los subdirectorios no... 

4. sin index en location, con autonidex off, da 404 y debe dar 403 forbiden 
(TESED con test_custom > GET /static/ )

5. Con directiva alias comentada, el acceso a un location va bien, con alias va mal.

6. @test/test_custom -> sh make_run.sh debug <-- LA FLAG debug 
    location /static/ {
        allow_methods GET;
        alias ./www/static;
        autoindex off;
    }
    CON FILE INDEX.HTML EN static/
    Aqui encuentra el index.html pero se va a ejecutar un CGI .js y peta
    (compilar con )


7.
    location /static/ {
        allow_methods GET;
        alias ./www/static;
        autoindex on;
    }

    No permite navegar a subdirectorios

8. no se esta comportando ok la logica de jerarquia para index.html > autoindex on/off > 404/403

9.
CRASH CONFIG SIN ;
CRASH UPLOADING FILE SIN: multipart, form-data


10. si la pagina de error no se encuentra se pasa a una default sin warning


*/
