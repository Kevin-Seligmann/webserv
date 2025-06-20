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
// void testRequestParser()
// {
//     uint8_t rawRequest[] =
//     "GET http://www.example.com?a's#fragm?ent  HTTP/1.0\r\n"
//     "Host: www.example.com\r\n"
//     "User-Agent: Mozilla/5.0\r\n"
//     "Content-Length: 7, 7, 7 \r\n"
//     "Content-Length: 7, 7\r\n"
//     "Accept: text/html\r\n"
//     "\r\n"
//     "BODYY"
//     "\r\n"
//     "GET /index.html HTTP/1.1\r\n"
//     "Host: www.example.com:8000\r\n"
//     "User-Agent: Mozilla/5.0\r\n"
//     "Accept: text/html\r\n"
//     "Transfer-Encoding: chunked\r\n"
//     "\r\n"
//     "1\r\n"
//     "a"
//     "1\r\n"
//     "b"
//     "1\r\n"
//     "e"
//     "0\r\n"
//     "\r\n"
//     "GET / HTTP/1.1\r\n"
//     "Host:                  \r\n"
//     "User-Agent: Mozillaaa/5.0\r\n"
//     "transfer-encoding: chunked\r\n"
//     "Accept: texttt/html\n\n"
//     "Qwerty is nice, really";

//     Logger & log = Logger::getInstance();
//     ErrorContainer error_container;
//     HTTPRequest request;
//     RequestValidator validator(request, error_container);
//     ElementParser element_parser(error_container);
//     RequestParser parser(request, error_container, element_parser, validator);

//     try 
//     {
//         for (int i = 0; rawRequest[i]; i++)
//         {
//             parser.append(rawRequest + i, 1);
//             if (error_container.error())
//                 continue ;
//             parser.process();
//             if (parser.done())
//             {
//                 log << request;
//                 error_container.log_all();   
//                 if(error_container.error())
//                     continue ;   
//                 error_container.reset(); 
//                 request.reset();
//                 parser.new_request();
//             }
//             if (error_container.error())
//                 error_container.log_all();
//         }

//         parser.dump_remainder(); 
//     }
//     catch (std::exception & e)
//     {
//         log.error(e.what());
//     }
// }

int main()
{
    // try 
    // {
    //     testLogger();
    //     testRequestParser();
    // }
    // catch (std::exception & e)
    // {
    //     std::cerr << "Server closed abruptly: " << e.what() << std::endl; 
    // }
}