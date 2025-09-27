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

===== SOLUION =====
- usar root ./www y dejar index index.html index.htm; porque alias se busca un archivo, no una carpeta
- usar "alias ./www/static;" y "index index.html index.htm;", en ese caso si hay index.html en la carpeta /static/, te muestra index, sino con autoindex on te muestra el arbol de la carpeta

variante con root:

location /static/ {
        allow_methods GET;
        root ./www;
        autoindex on;
    }


variante con alias

location /static/ {
        allow_methods GET;
        alias ./www/static;
        index index.html index.htm;
        autoindex on;
    }

===== END OF SOLUCION =====


7.
    location /static/ {
        allow_methods GET;
        alias ./www/static;
        autoindex on;
    }

    No permite navegar a subdirectorios

8. no se esta comportando ok la logica de jerarquia para index.html > autoindex on/off > 404/403

9. si pruebo POST request devuelva 
[DEBUG] parseFromCGIOutput: total bytes = 0
[Sat Sep 27 17:48:39 2025] Debug. CGI set status 2 CGI FINISHED
[Sat Sep 27 17:48:39 2025] Debug. Generating response for client 6
[Sat Sep 27 17:48:39 2025] Debug. Generating CGI response.
[DEBUG] SCRIPT_NAME=/cgi-bin/form.py
[DEBUG] SCRIPT_FILENAME=/home/irozhkov/ws_repo/test/test_custom/www/cgi-bin/form.py
[DEBUG] INTERPREETER=/usr/bin/python3
Exception ignored in: <_io.TextIOWrapper name='<stdout>' mode='w' encoding='utf-8'>
BrokenPipeError: [Errno 32] Broken pipe...

[Sat Sep 27 17:48:39 2025] Debug. Client 6 set status 0 Idle
[Sat Sep 27 17:50:34 2025] Debug. Client 6: connection closed by peer

10. si pruebo acceder directamente con http://localhost:8081/form.py (sin /cgi-bin) no salta a cgi-bin/form.py y me da esto

[DEBUG] SCRIPT_NAME=/form.py
[DEBUG] SCRIPT_FILENAME=/home/irozhkov/ws_repo/test/test_custom/www/form.py
[DEBUG] INTERPREETER=/usr/bin/python3
/usr/bin/python3: can't open file '/home/irozhkov/ws_repo/test/test_custom/www/form.py': [Errno 2] No such file or directory
[DEBUG] parseFromCGIOutput: total bytes = 0[Sat Sep 27 17:53:49 2025] Debug. CGI set status 2 CGI FINISHED

===== SOLUTION =====

He anadido una funcion al CGI.cpp:
std::string systemPathToCgi(const std::string &system_path);

tambien he hecho cambios de SCRIPT_NAME en std::map<std::string, std::string> CGI::pathToBlocks(const HTTPRequest& req) const

si en el path no hay cgi-bin - lo anada, p.e. http://localhost:8081/form.py se convierta a 
[DEBUG] SCRIPT_NAME=/cgi-bin/form.py
[DEBUG] SCRIPT_FILENAME=/home/irozhkov/ws_repo/test/test_custom/www/cgi-bin/form.py

entonces usamos una regla estricta: todos los scripts se ejecuta a traves cgi-bin, si no hay script [nombre].[extencion] - devuelva 404

===== END OF SOLUTION =====

11. upload no funciona, http://localhost:8081/cgi-bin/upload.py siempre da el mismo error
Error with status: Unsupported Media-Type (415)

9.
CRASH CONFIG SIN ;
CRASH UPLOADING FILE SIN: multipart, form-data

*/
