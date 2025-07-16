#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "HTTPRequest.hpp"
#include <string>
#include <vector>
#include <map>

// TODO: La clase Location no está terminada - falta implementar:
// - Error pages específicas por location (jerarquía: location -> server -> default)
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
	enum MatchType { EXACT, PREFIX };

private:
	std::string								_path;
	MatchType								_match_type;
	std::vector<std::string>				_methods;
	std::string								_root;
	std::string								_index;
	bool									_directory_listing;
	std::string                             _redirect;
	std::string 							_cgi_extension;
	std::string                             _upload_path;

public:
	Location();
	Location(const Location& other);
	~Location();

	bool matchesPath(const std::string& path) const;

	const std::string& getPath() const { return _path; }
	const MatchType& getMatchType() const { return _match_type; }
	const std::string& getRoot() const { return _root; }
    const std::string& getIndex() const { return _index; }
    const std::vector<std::string>& getMethods() const { return _methods; }
    bool hasDirectoryListing() const { return _directory_listing; }
    const std::string& getRedirect() const { return _redirect; }
    const std::string& getCgiExtension() const { return _cgi_extension; }
    const std::string& getUploadPath() const { return _upload_path; }

	void setPath(const std::string& path) { _path = path; }
	void setMatchType(const MatchType& match_type) { _match_type = match_type; }
    void setMethods(const std::vector<std::string>& methods) { _methods = methods; }
    void setRoot(const std::string& root) { _root = root; }
	void setIndex(const std::string& index) { _index = index; }
	void setDirectoryListing(const bool dl) { _directory_listing = dl; }
	void setRedirect(const std::string& redirect) { _redirect = redirect; }
	void setCgixtension(const std::string cgi_extension) { _cgi_extension = cgi_extension; }
	void setUploadPath(const std::string& upload_path) { _upload_path = upload_path; }
};

#endif