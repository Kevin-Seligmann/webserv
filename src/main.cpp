#include <iostream>
#include <exception>

#include "Logger.hpp"
#include "HTTPRequest.hpp"
#include "CGI.hpp"

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
/*    "GET http://host/../../..////././/a.b././%20%50c///d./././ef?a's#fragm?ent HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\r\n"
    "\n\n"*/
/*    "GET http://helo:8000/index?hola#chau HTTP/1.1\r\n"
    "Host: www.exampleee.com:7070\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\n\n"*/
/*    ""
    "GET / HTTP/1.1\r\n"
    "Host: www..com\r\n"
    "User-Agent: Mozillaaa/5.0\r\n"
    "Host: www..com\r\n"
    "Accept: texttt/html\n\n"
	""*/
    "GET /cgi-bin/hello.py?name=ilya&lang=ru HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\r\n"
    "\r\n"
	"POST /cgi-bin/submit.cgi HTTP/1.1\r\n"
    "Host: submit.example.com\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Content-Length: 27\r\n"
	"\r\n"
    "username=test&password=1234";
/*	"Qwerty is nice, really";*/

    Logger & log = Logger::getInstance();
    ErrorContainer error_container;
    HTTPRequest request;
    RequestValidator validator(request, error_container);
    ElementParser element_parser(error_container);
    RequestParser parser(request, error_container, element_parser, validator);

    try 
    {
//		std::cout << request << std::endl;
        for (int i = 0; rawRequest[i]; i++)
        {
            parser.append(rawRequest + i, 1);
            if (error_container.error())
                continue ;
            parser.process();
//			std::cout << request << std::endl;
            if (parser.done())
            {
                log << request;

				CGI cgi_test(request);
				cgi_test.runCGI();

                error_container.log_all();      
                error_container.reset(); 
                request.reset();
                parser.new_request();
            }
            if (error_container.error())
                error_container.log_all();
        }
		std::cout << request << std::endl;
        parser.dump_remainder(); 
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
