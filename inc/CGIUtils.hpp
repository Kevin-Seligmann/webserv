/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIUtils.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/14 19:12:20 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/30 20:33:53 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <arpa/inet.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <vector>

#include "Connection.hpp"
#include "HTTPMethod.hpp"

namespace CGIUtils
{
	std::string methodToString(HTTPMethod method);
	std::string socketToPort(int socket_fd);
	std::string intToString(int value);
	std::string getExtension(const std::string& path);
	char** reqToEnvp(const std::map<std::string, std::string>& src);
	char** reqToArgs(const std::map<std::string, std::string>& src, const std::map<std::string, std::string>& map); 
	void freeEnvp(char** envp);
}
