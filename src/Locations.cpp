#include "../inc/Locations.hpp"

// SETTERS Y GETTERS
void Locations::setPath(const std::string &path) { this->path = path; }

const std::string& Locations::getPath(void) const { return path; }

void Locations::setValidMethods(const std::vector<std::string> &validMethods) { allow_methods = validMethods; }

const std::vector<std::string>& Locations::getValidMethods(void) const { return allow_methods; }

void Locations::setRoot(const std::string &root) { this->root = root; }

const std::string& Locations::getRoot(void) const { return root; }

void Locations::setAutoindex(bool autoindex) { this->autoindex = autoindex; }

bool Locations::getAutoindex(void) const { return autoindex; }

std::string Locations::getCgiExtension(void) const {
    if (!cgi.empty()) {
        std::map<std::string, std::string>::const_iterator it = cgi.begin();
        return it->first;
    }
    return "";
}

std::string Locations::getCgiPath(void) const {
    if (!cgi.empty()) {
        std::map<std::string, std::string>::const_iterator it = cgi.begin();
        return it->second;
    }
    return "";
}

void Locations::setRedir(const std::string &redir) { return_path = redir; }

const std::string& Locations::getRedir(void) const { return return_path; }

void Locations::setUploadDir(const std::string &uploadDir) { upload_dir = uploadDir; }

const std::string& Locations::getUploadDir(void) const { return upload_dir; }
