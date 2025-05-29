#ifndef LOCATIONS
#define LOCATIONS

#include <string>
#include <string>
#include "Parsed.hpp"

class Locations {

    private:
        std::string                 _path;          // ruta lógica (request)
        std::vector<std::string>    _validMethods;  // default "GET"
        std::string                 _root;          // ruta física
        bool                        _autoindex;     // lista files si no hay index
        std::string                 _cgiExtension;  // extenión que llama a CGI
        std::string                 _cgiPath;       // ruta a interprete (app CGI)
        std::string                 _redir;         // url de redirect
        std::string                 _uploadDir;     // destino subidas

    public:
        Locations(const ParsedLocations &params);
        ~Locations();

        void setPath(const std::string &path);
        const std::string& getPath(void) const;

        void setValidMethods(const std::vector<std::string> &validMethods);
        const std::vector<std::string>& getValidMethods(void) const;

        void setRoot(const std::string &root);
        const std::string& getRoot(void) const;

        void setAutoindex(bool autoindex);
        bool getAutoindex(void) const;

        void setCgiExtension(const std::string &cgiExtension);
        const std::string& getCgiExtension(void) const;

        void setCgiPath(const std::string &cgiPath);
        const std::string& getCgiPath(void) const;

        void setRedir(const std::string &redir);
        const std::string& getRedir(void) const;

        void setUploadDir(const std::string &uploadDir);
        const std::string& getUploadDir(void) const;
};

#endif