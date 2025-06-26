/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIUtils.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/14 19:12:20 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/23 17:09:40 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "HTTPMethod.hpp"

namespace CGIUtils
{
	std::string methodToString(HTTPMethod method);
	std::string intToString(int value);
	std::string getExtension(const std::string& path);
	char** reqToEnvp(const std::map<std::string, std::string>& src);
	char** reqToArgs(const std::map<std::string, std::string>& src, const std::map<std::string, std::string>& map); 
	void freeEnvp(char** envp);
}
