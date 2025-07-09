/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:58:20 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/12 14:21:11 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

// Temporary color macros to fix compilation
#define RED        "\033[1;91m"
#define GREEN      "\033[1;92m"
#define YELLOW     "\033[1;93m"
#define BLUE       "\033[1;94m"
#define RESET      "\033[0m"

int checkFile(const char* argv)
{
    std::ifstream infile(argv);

    if (!infile.is_open())
    {
        std::cerr << std::endl << RED <<
        "ERROR: file does not exist or can not be opened." <<
        RESET << std::endl << std::endl;
        return (-1);
    }

    infile.seekg(0, std::ios_base::end);
    if (infile.tellg() == 0)
    {
        std::cerr << std::endl << RED <<
        "ERROR: file is empty." <<
        RESET << std::endl << std::endl;
        return (-1);
    }
    infile.seekg(0, std::ios_base::beg);

    if (infile.fail())
    {
        std::cerr << std::endl << RED <<
        "ERROR: file stream error detected." <<
        RESET << std::endl << std::endl;
        return (-1);
    }

    if (infile.is_open()) { infile.close(); }

    return (0);
}

void printLocationConfig(const Location& loc)
{
	// TODO: Implement when Location class has proper getters
	(void)loc; // Suppress unused parameter warning
	std::cout << "    Location configuration (not implemented yet)" << std::endl;
}

void printServerConfig(const ParsedServer& config)
{

	std::cout << GREEN << "\n=== ServerConfig ===" << RESET << std::endl;

	std::cout << YELLOW << "Listen Directives:" << RESET << std::endl;
	for (size_t i = 0; i < config.listens.size(); ++i)
		std::cout << "  - " << config.listens[i].to_string() << std::endl;

	std::cout << YELLOW << "Server Names:" << RESET << std::endl;
	for (size_t i = 0; i < config.server_names.size(); ++i)
		std::cout << "  - " << config.server_names[i] << std::endl;

	std::cout << YELLOW << "Root: " << RESET << config.root << std::endl;
	
	std::cout << YELLOW << "Error Pages:" << RESET << std::endl;
	for (std::map<int, std::string>::const_iterator it = config.error_pages.begin();
		 it != config.error_pages.end(); ++it)
		std::cout << "  " << it->first << " => " << it->second << std::endl;

	std::cout << YELLOW << "Allowed Methods:" << RESET << std::endl;
	for (size_t i = 0; i < config.allow_methods.size(); ++i)
		std::cout << "  - " << config.allow_methods[i] << std::endl;

	std::cout << YELLOW << "Autoindex: " << RESET << (config.autoindex ? "true" : "false") << std::endl;

	std::cout << YELLOW << "Client Max Body Size: " << RESET << config.client_max_body_size << std::endl;

	std::cout << YELLOW << "Locations:" << RESET << std::endl;
	for (std::map<std::string, Location>::const_iterator it = config.locations.begin();
		 it != config.locations.end(); ++it)
	{
		std::cout << "  Location block for: " << it->first << std::endl;
		printLocationConfig(it->second);
	}

	std::cout << std::endl;
}

void printParsingMessage(ParsingMessageType type)
{
	switch (type)
	{
		case DEFAULT_SERVER:
			std::cout << YELLOW << "INFO: " << RESET <<
			"According to the subject requirements, if two or more servers are configured " <<
			"with the same host:port, the first one in order is considered the primary. The " <<
			YELLOW << "[default_server]" << RESET << " directive is " << YELLOW <<
			"ignored" << RESET << "." << std::endl;
			break;
		case IPV6_HOST:
			std::cout << YELLOW << "INFO: " << RESET <<
			"According to the subject requirements, we accept IPv4 host form only. Hosts like " <<
			YELLOW << "[::] or [::1]" << RESET << " will be " << YELLOW <<
			"replaced with default host. " << RESET <<
			"According to subject, if this replace will affect same host:port of other server, " <<
			YELLOW << "the first one in order is considered the primary" << RESET << "." << std::endl;
			break;
		case LOCAL_HOST:
			std::cout << YELLOW << "INFO: " << RESET <<
			"Hosts like " << YELLOW << "localhost" << RESET << " will be " << YELLOW <<
			"replaced with the real localhost. " << RESET <<
			"According to subject, if this replace will affect same host:port of other server, " <<
			YELLOW << "the first one in order is considered the primary" << RESET << "." << std::endl;
			break;
		case ASTERIKS_HOST:
			std::cout << YELLOW << "INFO: " << RESET <<
			"Hosts like " << YELLOW << "*" << RESET << " will be " << YELLOW <<
			"replaced with default host. " << RESET <<
			"According to subject, if this replace will affect same host:port of other server, " <<
			YELLOW << "the first one in order is considered the primary" << RESET << "." << std::endl;
			break;
		default:
			std::cout << YELLOW << "INFO: " << RESET << "Unknown parsing state." << std::endl;
			break;
	}
}

std::string getLoopbackAddress()
{
	struct ifaddrs* ifaddr;
	if (getifaddrs(&ifaddr) == -1)
		return ("0.0.0.0");

	std::string loopbackIP = "127.0.0.1";
	for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (!ifa->ifa_addr)
			continue;

		if (ifa->ifa_addr->sa_family == AF_INET && std::string(ifa->ifa_name) == "lo")
		{
			char addrBuf[INET_ADDRSTRLEN];
			void* addrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
			inet_ntop(AF_INET, addrPtr, addrBuf, INET_ADDRSTRLEN);
			loopbackIP = std::string(addrBuf);
			break;
		}
	}

	freeifaddrs(ifaddr);
	return (loopbackIP);
}

/* Logs Utility */

void OKlogsEntry(const std::string& title, const std::string& str) {
	std::cout << BLUE << title << RESET << str << std::endl;
}

void ERRORlogsEntry(const std::string& title, const std::string& str) {
	std::cout << RED << title << RESET << str << std::endl;
}

