#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
public:
    std::string                         method;
    std::string                         path;
    std::map<std::string, std::string>  headers;
    std::string                         body;

    HttpRequest();
    ~HttpRequest();
};

#endif