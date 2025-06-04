#ifndef LOCATIONS_HPP
#define LOCATIONS_HPP

#include <string>
#include <vector>
#include <map>

struct Locations
{
	std::string								path;
	std::string								root;
	std::string								index;
	std::vector<std::string>				allow_methods;
	bool									autoindex;
	std::string                             return_path;
	bool                                    allow_upload;
	std::string                             upload_dir;
	std::map<std::string, std::string>      cgi;

	Locations()
		: autoindex(false), allow_upload(false) {}
		
	// SETTERS GETTERS

	void setPath(const std::string &path);
	const std::string& getPath(void) const;

	void setValidMethods(const std::vector<std::string> &validMethods);
	const std::vector<std::string>& getValidMethods(void) const;

	void setRoot(const std::string &root);
	const std::string& getRoot(void) const;

	void setAutoindex(bool autoindex);
	bool getAutoindex(void) const;

	std::string getCgiExtension(void) const;
	std::string getCgiPath(void) const;

	void setRedir(const std::string &redir);
	const std::string& getRedir(void) const;

	void setUploadDir(const std::string &uploadDir);
	const std::string& getUploadDir(void) const;
};

#endif