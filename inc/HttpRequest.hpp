// esta clase solo tiene el fin de servir para implementar la funcion de clase matchRequestAndServer

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
public:
    std::string method;  // GET etc
    std::string path;    // recurso solicitado
    std::map<std::string, std::string> headers; // cabeceras
    std::string body;    // cuerpo

    HttpRequest();
    ~HttpRequest();
};

#endif
