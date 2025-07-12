#include <sstream>
#include <cstdlib>  // Para std::atoi
#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest(){}

void HTTPRequest::reset()
{
    body.reset();
    uri.reset();
    headers.reset();
    method = NOMETHOD;
    protocol = "";
}

std::string HTTPRequest::to_string()
{
    std::stringstream ss;

    print(ss);
    return ss.str();
}

void HTTPRequest::print(std::ostream & os) const
{
    os 
    << "METHOD: " << method << "\n"
    << "URI: " << uri << "\n"
    << "PROTOCOL: " << protocol << "\n"
    << "HEADERS: " << "\n" << headers
    << "BODY: " << body << "\n";
}

std::ostream & operator<<(std::ostream & os, HTTPRequest request)
{
    request.print(os);
    return os;
}

int HTTPRequest::get_port() const
{
    // Return port from URI, or default to 80 if not specified
    if (uri.port == 0)
        return 80;
    else
        return uri.port;
}

std::string const HTTPRequest::get_host() const
{
    // Try to get host from Host header first, then from URI
    std::map<std::string, std::string>::const_iterator it = headers.fields.find("Host");
    if (it != headers.fields.end())
    {
        std::string host_header = it->second;
        // Remove port from host header if present (host:port format)
        size_t colon_pos = host_header.find(':');
        if (colon_pos != std::string::npos)
            return host_header.substr(0, colon_pos);
        return host_header;
    }
    
    // Alternative: use the host field directly
    if (!headers.host.empty())
        return headers.host;
    
    // Fallback to URI host
    if (!uri.host.empty())
        return uri.host;
        
    // Default fallback
    return "localhost";
}

std::string const HTTPRequest::get_path() const
{
    if (uri.path.empty())
        return "/";
    else
        return uri.path;
}
