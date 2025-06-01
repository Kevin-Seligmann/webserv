#include <iostream>
#include <exception>

#include "Logger.hpp"
#include "HTTPRequest.hpp"

void testLogger()
{
    Logger & log = Logger::getInstance();
    log.info("Bienvenido a Webserv");
    log.warning("In development");
    log.error("Nothing else done");
}

#include "RequestParser.hpp"
void testRequestParser()
{
    uint8_t rawRequest[] =
    "GET /index.html HTTP/1.1   \r\n"
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\r\n"
    "\n\n"
    "GET /indexxx.html HTTP/1.1    \r\n"
    "Host: www.exampleee.com\r\n"
    "User-Agent: Mozillaaa/5.0\r\n"
    "Accept: texttt/html\n\n"
    ""
    "GET / HTTP/1.1\r\n"
    "Host: www..com\r\n"
    "User-Agent: Mozillaaa/5.0\r\n"
    "Accept: texttt/html\n\n"
    "Qwerty is nice, really";

    Logger & log = Logger::getInstance();
    HTTPRequest request;
    RequestParser parser(request);
    try 
    {
        for (int i = 0; rawRequest[i]; i++)
        {
            parser.append(rawRequest + i, 1);
            parser.process();
            if (parser.done())
            {
                log << request;
                parser.new_request();
            }
        }
        parser.dump_remainer();        
    }
    catch (std::exception & e)
    {
        log.error(e.what());
    }
}

int main()
{
    try 
    {
        testLogger();
        testRequestParser();
    }
    catch (std::exception & e)
    {
        std::cerr << "Server closed abruptly: " << e.what() << std::endl; 
    }
}