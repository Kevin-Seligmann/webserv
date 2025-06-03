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
		
	// Methods that were in the Locations class
	void setPath(const std::string &path) { this->path = path; }
	const std::string& getPath(void) const { return path; }

	void setValidMethods(const std::vector<std::string> &validMethods) { allow_methods = validMethods; }
	const std::vector<std::string>& getValidMethods(void) const { return allow_methods; }

	void setRoot(const std::string &root) { this->root = root; }
	const std::string& getRoot(void) const { return root; }

	void setAutoindex(bool autoindex) { this->autoindex = autoindex; }
	bool getAutoindex(void) const { return autoindex; }

	// CGI helper methods
	std::string getCgiExtension(void) const {
		if (!cgi.empty()) {
			std::map<std::string, std::string>::const_iterator it = cgi.begin();
			return it->first;
		}
		return "";
	}
	
	std::string getCgiPath(void) const {
		if (!cgi.empty()) {
			std::map<std::string, std::string>::const_iterator it = cgi.begin();
			return it->second;
		}
		return "";
	}

	void setRedir(const std::string &redir) { return_path = redir; }
	const std::string& getRedir(void) const { return return_path; }

	void setUploadDir(const std::string &uploadDir) { upload_dir = uploadDir; }
	const std::string& getUploadDir(void) const { return upload_dir; }
};

#endif
