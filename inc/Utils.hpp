/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:51:01 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/04 19:29:12 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# pragma once

# include <arpa/inet.h>
# include <cstring>
# include <fstream>
# include <ifaddrs.h>
# include <iostream>
# include <map>
# include <netinet/in.h>
# include <sstream>
# include <stdexcept>
# include <string>
# include <sys/socket.h>
# include <unistd.h>
# include <vector>

# include "Parsed.hpp"
# include "ServersManager.hpp"
# include "SocketsManager.hpp"

/* Colores para debug y otros */
# define RED        "\033[1;91m" 
# define GREEN      "\033[1;92m"  
# define YELLOW     "\033[1;93m"
# define BLUE       "\033[1;94m"
# define RESET      "\033[0m"

/* Check input file function */
int checkFile(const char* argv);

/* Function for getting real localhost */
std::string getLoopbackAddress();

/* Functions for printing */
void printLocationConfig(const Locations& loc);
void printServerConfig(const ParsedServer& config);

/* Server initialization functions */
bool serversInit(ServersManager& sm, const ParsedServers& ps);
bool socketsInit(SocketsManager listenSockets, ServersManager webServers);

#endif
