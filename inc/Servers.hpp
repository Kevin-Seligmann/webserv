#ifndef SERVERS_HPP
#define SERVERS_HPP

#include <string>
#include <vector>
#include <map>
#include "Locations.hpp"
#include "Parsed.hpp"
#include "HostPort.hpp"

class Servers {

	private:
		HostPort					_hostPort;		// objeto HostPort usado como key
		std::string         		_host; 			// direccion IP
		int                 		_port;			// puerto de escuha
		std::string					_server_name;	// nombre del server
		std::string                 _root;			// directorio raiz
		std::vector<std::string>	_indexFiles;	// index.html etc.
		std::map<int, std::string>	_errorPages;	// 404.html etc.
		std::vector<Locations>		_locations;		// instancia de Location
		bool						_default_server; // por defecto false

	public:
		Servers(const ParsedServer &params, bool default_server = false);
		~Servers();

		void setHostPort(const HostPort &hostPort);
		const HostPort& getHostPort(void) const;

		void setHost(const std::string &host);
		const std::string& getHost(void) const;
		
		void setPort(const int port);
		int getPort(void) const;

		void setServerName(const std::string &serverName);
		const std::string& getServerName(void) const;

		void setRoot(const std::string &root);
		const std::string& getRoot(void) const;

		void setIndexFiles(const std::vector<std::string> &indexFiles);
		const std::vector<std::string>& getIndexFiles(void) const;

		void setErrorPages(const std::map<int, std::string> &errorPages);
		const std::map<int, std::string>& getErrorPages(void) const;

		void setLocations(const std::vector<Locations>& locations);
		const std::vector<Locations>& getLocations(void) const;

		void setDefaultServer(bool val);
		bool isDefaultServer() const;
};


#endif