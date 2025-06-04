#ifndef SERVERS_HPP
#define SERVERS_HPP

#include <string>
#include <vector>
#include <map>
#include "Parsed.hpp"
#include "Listen.hpp"

class Servers {

	private:
		Listen								_listen;
		std::vector<std::string>			_server_names;
		std::string                 		_root;
		std::vector<std::string>			_indexFiles;
		std::map<int, std::string>			_errorPages;
		std::vector<std::string>			_allow_methods;
		bool								_autoindex;
		std::string							_client_max_body_size;
		std::map<std::string, Locations>	_locations;

	public:
		Servers(const ParsedServer &params);
		Servers(const ParsedServer &params, const Listen &listen);
		~Servers();

		// GETTERS SETTERS
		
		void setListen(const Listen &listen);
		const Listen& getListens(void) const;

		void setServerNames(const std::vector<std::string> &serverNames);
		const std::vector<std::string>& getServerNames(void) const;
		
		void setRoot(const std::string &root);
		const std::string& getRoot(void) const;

		void setIndexFiles(const std::vector<std::string> &indexFiles);
		const std::vector<std::string>& getIndexFiles(void) const;

		void setErrorPages(const std::map<int, std::string> &errorPages);
		const std::map<int, std::string>& getErrorPages(void) const;

		void setAllowMethods(const std::vector<std::string> &allowMethods);
		const std::vector<std::string>& getAllowMethods(void) const;

		void setAutoindex(bool autoindex);
		bool getAutoindex(void) const;

		void setClientMaxBodySize(const std::string &clientMaxBodySize);
		const std::string& getClientMaxBodySize(void) const;

		void setLocations(const std::map<std::string, Locations>& locations);
		const std::map<std::string, Locations>& getLocations(void) const;
};

#endif