/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIError_check.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/02 12:33:56 by irozhkov          #+#    #+#             */
/*   Updated: 2025/10/02 13:01:54 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIError_check.hpp"

bool hasContentType(const std::string& headers)
{
	std::istringstream iss(headers);
	std::string line;

	while (std::getline(iss, line))
	{
        if (!line.empty() && line[line.size() - 1] == '\r')
		{
            line.erase(line.size() - 1);
		}
        
		std::string lower = line;
		for (size_t i = 0; i < lower.size(); ++i)
		{
			lower[i] = static_cast<char>(std::tolower(lower[i]));
		}

		if (lower.find("content-type:") == 0)
		{
			return true;
		}
	}

	return false;
}

int cgiErrorCheck(const std::string& out)
{

	if (out.empty()) { return 502; }
	else if (out.find("__CGI_ERROR_404__") != std::string::npos) { return 404; }
	else if (out.find("__CGI_ERROR_403__") != std::string::npos) { return 403; }
	else if (out.find("__CGI_ERROR_500__") != std::string::npos) { return 500; }

	size_t pos = out.find("\r\n\r\n");

    if (pos == std::string::npos) { return 502; }

    std::string headers = out.substr(0, pos);
    if (!hasContentType(headers)) { return 502; }

	return 200;
}
