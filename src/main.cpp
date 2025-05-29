#include <iostream>
#include <exception>

#include "Logger.hpp"
#include "HTTPRequest.hpp"

int main()
{
    try 
    {
        Logger & log = Logger::getInstance();
        log.info("Bienvenido a Webserv");
        log.warning("In development");
        log.error("Nothing else done");

        HTTPRequest rq;
        log << rq;
    }
    catch (std::exception & e)
    {
        std::cerr << "Server closed abruptly: " << e.what() << std::endl; 
    }
}