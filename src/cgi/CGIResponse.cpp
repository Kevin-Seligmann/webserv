/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvisca-g <mvisca-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 11:38:25 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/24 19:22:35 by mvisca-g         ###   ########.fr       */
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
	std::map<std::string, std::string>::const_iterator it = _cgiResponseHeaders.find(key);
	if (it != _cgiResponseHeaders.end())
		return (it->second);
	return ("");
}

const std::map<std::string, std::string>& CGIResponse::getCGIResponseHeaders() const
{
	return (_cgiResponseHeaders);
}

const std::string& CGIResponse::getResponseBuffer() const
{
	return (_responseBuffer);
}

void CGIResponse::parseFromCGIOutput(const std::string& cgiOutput)
{
	DEBUG_LOG("parseFromCGIOutput: total bytes = " << cgiOutput.size());
	
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
			DEBUG_LOG("KEY value: " << key);
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
				_cgiResponseHeaders[key] = value;
			}
		}
		else
		{
			bodyTmp << line;
		}
	}

	std::string bodyStr = bodyTmp.str();
	if (!bodyStr.empty() && bodyStr[bodyStr.size() - 1] == '\n')
	{
		bodyStr.erase(bodyStr.size() - 1, 1);
	}

	_bodyStream << bodyStr;

	if (_status.empty()) { _status = "200 OK"; } // TODO no confiable debe ser dinamico
	if (_contentType.empty()) { _contentType = "text/plain"; }
}


void CGIResponse::buildResponse()
{
	std::ostringstream response;

	close = false;

	response << "HTTP/1.1 " << (_status.empty() ? "200 OK" : _status) << "\r\n";

	// response << "Content-Type: " << (_contentType.empty() ? "text/plain" : _contentType) << "\r\n";

	if (!_location.empty())
		response << "Location: " << _location << "\r\n";

	for (std::map<std::string,std::string>::const_iterator it = _cgiResponseHeaders.begin();
		 it != _cgiResponseHeaders.end(); ++it)
	{
		response << it->first << ": " << it->second << "\r\n";
	}

	std::string body = _bodyStream.str();

	_bodyStream.str("");
	_bodyStream.clear();

	response << "Content-Length: " << body.size() << "\r\n";
	
	response << "\r\n";
	response << body;

    _responseBuffer = response.str();

	_sentBytes = 0;
}

void CGIResponse::buildStreamedResponse()
{
	std::ostringstream response;

	response << "HTTP/1.1 " << (_status.empty() ? "200 OK" : _status) << "\r\n";

	// response << "Content-Type: " << (_contentType.empty() ? "text/plain" : _contentType) << "\r\n";

	if (!_location.empty())
		response << "Location: " << _location << "\r\n";

	for (std::map<std::string,std::string>::const_iterator it = _cgiResponseHeaders.begin();
		 it != _cgiResponseHeaders.end(); ++it)
	{
		response << it->first << ": " << it->second << "\r\n";
	}

	std::string body = _bodyStream.str();

	_bodyStream.str("");
	_bodyStream.clear();

	if (!getCGIResponseHeader("Content-Length").empty())
	{
		response << "Content-Length: " << getCGIResponseHeader("Content-Length") << "\r\n";
		close = false;
	}
	else 
	{
		response << "Connection: close" << "\r\n";
		close = true;
	}

	response << "\r\n";
	response << body;

    _responseBuffer = response.str();

	_sentBytes = 0;
}

void CGIResponse::buildInternalErrorResponse()
{
	_status = "500 Internal Server Error";
	_contentType = "text/plain";
	_location.clear();
	_cgiResponseHeaders.clear();
	_bodyStream.str("");
	_bodyStream << "Internal Server Error";

	buildResponse();
}

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
	std::map<std::string, std::string>::const_iterator it = _cgiResponseHeaders.find(header);
    if (it != _cgiResponseHeaders.end())
		throw std::runtime_error("This header already exist: " + header);
    _cgiResponseHeaders[header] = value;
}

void CGIResponse::reset()
{
	std::string().swap(_contentType);
	std::string().swap(_status);
	std::string().swap(_location);
	std::string().swap(_responseBuffer);

	_cgiResponseHeaders.clear();
	std::map<std::string, std::string>().swap(_cgiResponseHeaders);

	_bodyStream.str("");
	_bodyStream.clear();
}