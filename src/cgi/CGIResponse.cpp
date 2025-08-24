/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 11:38:25 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/24 16:25:36 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIResponse.hpp"

std::string CGIResponse::getContentType() const
{
	return (_contentType);
}

std::string CGIResponse::getStatus() const
{
	return  (_status);
}

std::string CGIResponse::getLocation() const
{
	return (_location);
}

std::string CGIResponse::getCGIResponseHeader(const std::string& key) const
{
	std::map<std::string, std::string>const_iterator it = _cgiResponseHeaders.find(key);
	if (it != _cgiResponseHeaders.end())
		return (it->second);
	return ("");
}

const std::map<std::string, std::string>& CGIResponse::getCGIResponseHeaders()
{
	return (_cgiResponseHeaders);
}

void parseFromCGIOutput(const std::string& cgiOutput)
{
	std::istringstream stream(cgiOutput);
	std::string line;

	bool headerDone = false;

	_contentType.clear();
	_status.clear();
	_location.clear();
	_cgiResponseHeaders.clear();
	_bodyStream.str("");

	std::ostringstream bodyTmp;

	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size()-1] == '\r')
		{
			line.erase(line.size()-1);
		}

		if (!headerDone && line.empty())
		{
			headerDone = true;
			continue;
		}

		if (!headerDone)
		{
			size_t pos = line.find(':');

			if (pos == std::string::npos) 
			{
				continue;
			}

			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			
			while (!value.empty() && (value[0]==' ' || value[0]=='\t'))
			{
				value.erase(0,1);
			}

			if (key == "Status") 
			{
				_status = value;
			}
			else if (key == "Content-Type") 
			{
				_contentType = value;
			}
			else if (key == "Location") 
			{
				_location = value;
			}
			else 
			{
				_CGIResponseHeaders[key] = value;
			}
		}
		else
		{
			bodyTmp << line << "\n";
		}
	}

	_bodyStream << bodyTmp.str();

	if (_status.empty()) _status = "200 OK";
	if (_contentType.empty()) _contentType = "text/plain";
}

/*
void buildResponse()
{
	std::
}
*/

void CGIResponse::setContentType(const std::string& type)
{
	_contentType = type;
}

void CGIResponse::setStatus(const std::string& stat)
{
	_status = stat;
}

void CGIResponse::setLocation(const std::string& loc)
{
	_location = loc;
}

void CGIResponse::setHeader(const std::string& header, const std::string& value)
{
	std::map<std::string, std::string>const_iterator it = _cgiResponseHeaders.find(header);
    if (it != _cgiResponseHeaders.end())
		throw std::runtime_error("This header already exist: " + header);
    _cgiResponseHeaders[header] = value;
}
