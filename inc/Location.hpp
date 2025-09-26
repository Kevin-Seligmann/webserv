#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Enums.hpp"
#include "StringUtil.hpp"
#include "HTTPRequest.hpp"
#include <string>
#include <vector>
#include <map>

// TODO: La clase Location no está terminada - falta implementar:
// - Validación completa de configuración
// - Manejo de herencia de configuración desde el server padre
// - Implementación completa de CGI configuration
// - CRITICAL: Manejo de exact match vs prefix match en locations:
//   * location = /     -> exact match para "/"
//   * location /       -> prefix match para todo lo que empiece con "/"
//   * Si request es "/index.html" y existe "location = /" con "index index.html"
//     debe hacer redirect INTERNO (no 301) para usar la location exacta
//   * El match maker debe resolver esto internamente sin informar al cliente
//   * Necesita re-matching automático cuando se resuelve un index file

class Location {
public:
	enum MatchType { EXACT, PREFIX, UNSET };

private:
	std::string								_path;
	MatchType								_match_type;
	std::vector<std::string>				_methods;
	std::string								_root;
	std::vector<std::string>				_index;
	AutoIndexState							_autoindex;
	std::string                             _redirect;
	std::string 							_cgi_extension;
	bool									_allow_upload;
	std::map<int, std::string> 				_error_pages;
	size_t 									_max_body_size;
	std::string 							_alias;

public:
	Location();
	Location(const Location& other);
	~Location();

	bool matchesPath(const std::string& path) const;

	const std::string& getPath() const { return _path; }

	const MatchType& getMatchType() const { return _match_type; }

	const std::string& getRoot() const { return _root; }

    const std::vector<std::string>& getIndex() const { return _index; }

	AutoIndexState getAutoindex() const { return _autoindex; }

    const std::vector<std::string>& getMethods() const { return _methods; }

    const std::string& getRedirect() const { return _redirect; }

    const std::string& getCgiExtension() const { return _cgi_extension; }

	const std::string & getAlias() const {return _alias;}

	bool getAllowUpload() const { return _allow_upload; }
	
	std::string getErrorPage(int error_code) const;

	size_t getMaxBodySize() const { return _max_body_size; }
	
    bool hasAutoindex() const { return _autoindex; }

	std::string getFilesystemLocation(std::string const & path) const;
	
	void setPath(const std::string& path) { _path = path; }
	void setMatchType(const MatchType& match_type) { _match_type = match_type; }
    void setMethods(const std::vector<std::string>& methods) { _methods = methods; }
    void setRoot(const std::string& root) { _root = wss::guarantee_absolute_path(root); }
	void setIndex(const std::vector<std::string>& index) { _index = index; }
	void setAutoindex(AutoIndexState state) { _autoindex = state; }
	void setRedirect(const std::string& redirect) { _redirect = redirect; }
	void setCgixtension(const std::string cgi_extension) { _cgi_extension = cgi_extension; }
	void setAllowUpload(const bool allow_upload) { _allow_upload = allow_upload; }
	void setErrorPage(int code, const std::string& path) { _error_pages[code]=path; }
	void setMaxBodySize(size_t max) { _max_body_size = max; }
	void setAlias(const std::string & alias){_alias = wss::guarantee_absolute_path(alias);}
};

#endif
