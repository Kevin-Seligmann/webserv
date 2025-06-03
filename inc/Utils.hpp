/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:51:01 by irozhkov          #+#    #+#             */
/*   Updated: 2025/05/31 12:44:00 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# pragma once

# include <fstream>
# include <iostream>
# include <map>
# include <sstream>
# include <stdexcept>
# include <string>
# include <vector>

# include "Parsed.hpp"
# include "ServersManager.hpp"
# include "SocketsManager.hpp"

// ==================== COLOR DEFINITIONS ====================
// Unified colors used in the project
# define RED        "\033[1;91m"   // Bright red for errors
# define GREEN      "\033[1;92m"   // Bright green for success  
# define YELLOW     "\033[1;93m"   // Bright yellow for warnings/highlights
# define BLUE       "\033[1;94m"   // Bright blue for info
# define RESET      "\033[0m"      // Reset to default

/* Check input file function */
int checkFile(const char* argv);

/* Functions for printing */
void printLocationConfig(const Locations& loc);
void printServerConfig(const ParsedServer& config);

/* Server initialization functions */
bool serversInit(ServersManager& sm, const ParsedServers& ps);
bool socketsInit(SocketsManager listenSockets, ServersManager webServers);

#endif
