/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIError_check.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/02 12:33:56 by irozhkov          #+#    #+#             */
/*   Updated: 2025/10/03 12:46:44 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIError_check.hpp"

bool hasContentType(const std::string& headers)
{
	std::istringstream iss(headers);
	std::string line;

	while (std::getline(iss, line))
	{
		if (!line.empty() && line[line.size()-1] == '\r')
			line.erase(line.size()-1);

		std::string lower = line;
		for (size_t i = 0; i < lower.size(); ++i)
			lower[i] = static_cast<char>(std::tolower(lower[i]));

		if (lower.find("content-type:") == 0)
			return true;
	}

	return false;
}

int cgiErrorCheck(const std::string& out)
{
	if (out.empty()) { return 502; }
	if (out.find("__CGI_ERROR_404__") != std::string::npos) { return 404; }
	if (out.find("__CGI_ERROR_403__") != std::string::npos) { return 403; }
	if (out.find("__CGI_ERROR_500__") != std::string::npos) { return 500; }

	size_t pos = out.find("\r\n\r\n");
	std::string headers;

	if (pos != std::string::npos) 
	{
		headers = out.substr(0, pos);
	} 
	else 
	{
		std::istringstream iss(out);
		std::string line;
		while (std::getline(iss, line)) {
			if (!line.empty() && line[line.size()-1] == '\r')
				line.erase(line.size()-1);
			if (line.empty()) break;
            headers += line + "\n";
		}
	}

	if (!hasContentType(headers)) { return 502; }

	return 200;
}
