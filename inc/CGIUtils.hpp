/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIUtils.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/14 19:12:20 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/16 18:01:52 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <map>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "HTTPMethod.hpp"

namespace CGIUtils
{
	std::string methodToString(HTTPMethod method);
	std::string getExtension(const std::string& path);
	char** reqToEnvp(const std::map<std::string, std::string>& src);
	char** reqToArgs(const std::map<std::string, std::string>& src, const std::map<std::string, std::string>& map); 
	void freeEnvp(char** envp);
}
