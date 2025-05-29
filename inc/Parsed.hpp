/* 
Structs para almacenar los valores del parseo.
Serán usados para crear las instancias de clase Servers (que contiene Locations)
El constructor de cada clase incluye los valores por defecto, de modo que todos los valores estén seteados al ser pasado al constructor de las clases Servers y Locations.
*/

#ifndef PARSED_HPP
#define PARSED_HPP

#include <string>
#include <vector>
#include <map>

struct ParsedLocations {
    std::string 				path;
    std::vector<std::string>	validMethods;
    std::string					root;
    bool						autoindex;
    std::string 				cgiExtension;
    std::string 				cgiPath;
    std::string 				redir;
    std::string 				uploadDir;

    ParsedLocations(const std::string& 				path,
					const std::vector<std::string>&	validMethods,
                    const std::string&				root,
					bool							autoindex,
                    const std::string&				cgiExtension,
					const std::string&				cgiPath,
                    const std::string&				redir,
					const std::string&				uploadDir);

    ~ParsedLocations();
};

struct ParsedServer {
    std::string						host;
    int								port;
    std::string						serverName;
    std::string						root;
    std::vector<std::string>		indexFiles;
    std::map<int, std::string>		errorPages;
    std::vector<ParsedLocations>	locations;

    ParsedServer(const std::string& 				host,
				int									port,
				const std::string&					serverName,
                const std::string&					root,
				const std::vector<std::string>&		indexFiles,
                const std::map<int, std::string>&	errorPages,
                const std::vector<ParsedLocations>&	locations);
	
    ~ParsedServer();
};

typedef std::vector<ParsedServer> ParsedServers;

#endif