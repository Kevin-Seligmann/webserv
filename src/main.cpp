#include "../inc/VirtualServersManager.hpp"
#include "../inc/Parsed.hpp"
#include "../inc/Utils.hpp"
#include "../inc/debug.hpp"
#include <iostream>

int main(int argc, char** argv) {
	try {
		ParsedServers config;
		if (parseProcess(argc, argv, config) != 0) return (1);
		showParse(config);

		VirtualServersManager webServers;
		if (!serversInit(webServers, config)) {
			std::cerr << RED << "Failed to initialize servers." << RESET << std::endl;
			return (1);
		}
		showServers(webServers);

		// =====================================================================
		// FASE 3: MAIN EVENT LOOP (FUTURO)
		// =====================================================================
		// TODO: Implementar event loop con epoll
		// 1. Crear epoll instance
		// 2. Registrar listen sockets en epoll usando punteros a VirtualServerKey
		// 3. Event loop principal para manejar conexiones
		
		std::cout << GREEN << "Server ready. Event loop will be implemented next." << RESET << std::endl;

		return (0);
	} catch (const std::runtime_error& e) {
		std::cerr << RED << "RUNTIME ERROR: " << RESET << e.what() << std::endl;
		return (1);
	} catch (const std::exception& e) {
		std::cerr << RED << "STD_EXCEPTION ERROR: " << RESET << e.what() << std::endl;
		return (1);
	} catch (...) {
		std::cerr << RED << "WIRED ERROR: " << RESET << "Unknown exception occurred" << std::endl;
		return (1);
	}
}


// #include <iostream>
// #include <exception>

// #include "Logger.hpp"
// #include "HTTPRequest.hpp"

// void testLogger()
// {
//     Logger & log = Logger::getInstance();
//     log.info("Bienvenido a Webserv");
//     log.warning("In development");
//     log.error("Nothing else done");
// }

// #include "RequestParser.hpp"
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
//         parser.dump_remainder(); 
//     }
//     catch (std::exception & e)
//     {
//         log.error(e.what());
//     }
// }

// int main()
// {
//     try 
//     {
//         testLogger();
//         testRequestParser();
//     }
//     catch (std::exception & e)
//     {
//         std::cerr << "Server closed abruptly: " << e.what() << std::endl; 
//     }
// }