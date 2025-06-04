/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:58:20 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/04 19:28:07 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

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

void printLocationConfig(const Locations& loc)
{
	std::cout << "    Path: " << loc.path << std::endl;
	std::cout << "    Root: " << loc.root << std::endl;
	std::cout << "    Index: " << loc.index << std::endl;
	std::cout << "    Autoindex: " << (loc.autoindex ? "true" : "false") << std::endl;
	std::cout << "    Return Path: " << loc.return_path << std::endl;
	std::cout << "    Allow Upload: " << (loc.allow_upload ? "true" : "false") << std::endl;
	std::cout << "    Upload Dir: " << loc.upload_dir << std::endl;

	std::cout << "    Allow Methods:" << std::endl;
	for (size_t i = 0; i < loc.allow_methods.size(); ++i)
		std::cout << "      - " << loc.allow_methods[i] << std::endl;

	std::cout << "    CGI Handlers:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it)
		std::cout << "      " << it->first << " => " << it->second << std::endl;
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
	for (std::map<std::string, Locations>::const_iterator it = config.locations.begin();
		 it != config.locations.end(); ++it)
	{
		std::cout << "  Location block for: " << it->first << std::endl;
		printLocationConfig(it->second);
	}

	std::cout << std::endl;
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
