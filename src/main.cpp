/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 12:09:35 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/02 18:27:30 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Parsed.hpp"
#include "ServerValidator.hpp"
#include "Utils.hpp"

int main(int argc, char* argv[])
{
	if (argc > 2)
	{
		std::cerr << std::endl << RED << "ERROR: " << DEFAULT <<
		"This program start with arguments ---> " << YELLOW <<
		"./webserver [filename]" << DEFAULT << std::endl << std::endl;
		return (1);
	}

	if (argc == 2)
		if (checkFile(argv[1]) == -1)
			return (1);

	try
	{
		std::ifstream file(argv[1]);
		std::stringstream buffer;
		buffer << file.rdbuf();

		std::string content = buffer.str();
		std::vector<std::string> tokens = tokenize(content);

		std::cout << "\n=== Tokenized file of config ===" << std::endl;
		std::vector<std::string>::iterator it;
		for (it = tokens.begin(); it != tokens.end(); ++it)
		{
			std::cout << *it << std::endl;
		}

		std::vector<ParsedServer> servers = parseConfig(tokens);

		for (size_t i = 0; i < servers.size(); ++i)
		{
			std::cout << "\n--- Server " << i + 1 << " ---\n";
			printServerConfig(servers[i]);
		}

		ServerValidator::validate(servers);
	}
	catch (const  std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}
