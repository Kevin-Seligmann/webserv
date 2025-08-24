/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 15:21:11 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/23 18:46:37 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI(const HTTPRequest& req, const ParsedServer& server)
{
	buildEnv(req, server);
//	pid = -1;

}

std::string CGI::methodToString(HTTPMethod method) const 
{
	switch (method)
	{
		case HTTPMethod::GET:
			return ("GET");
		case HTTPMethod::POST:
			return ("POST");
		case HTTPMethod::PUT:
			return ("PUT");
		case HTTPMethod::DELETE:
			return ("DELETE");
		default:
			return ("UNKNOWN");
	}
}

void CGI::buildEnv(const HTTPRequest& req, const ParsedServer& server)
{
	if (!req.body.empty())
	{
		_env.setEnvValue("CONTENT_LENGTH", std::to_string(req.body.size()));
//		auto it = req.headers.find("Content-Type");
//		if (it != req.headers.end())
//			_env["CONTENT_TYPE"] = it->second;
	}
	
	_env.setEnvValue("GATEWAY_INTERFACE", "CGI/1.1"); // por defecto

	if (req.uri.query.empty()) { _env.setEnvValue("QUERY_STRING", ""); }
	else { _env.setEnvValue("QUERY_STRING", req.uri.query); }

	_env.setEnvValue("PATH_INFO", req.uri.path); // parte de uri despues del nombre de script!!!

	_env.setEnvValue("REQUEST_METHOD", methodToString(req.method));
	_env.setEnvValue("SCRIPT_NAME", script_path); // TODO check what is here
	_env.setEnvValue("SERVER_NAME", "server"); // TODO server_config
	_env.setEnvValue("SERVER_PORT", "8080"); // TODO CGIUtils::socketToPort(con.getSocketFd());
	_env.setEnvValue("SERVER_PROTOCOL", req.protocol);
	_env.setEnvValue("SERVER_SOFTWARE", "webserver"); // este variable es obligatorio, pero no influye a script call
 
}

CGIEnv& CGI::getEnv()
{
    return (_env);
}
