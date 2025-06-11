/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/07 16:14:39 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/10 12:14:04 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

/*
	Nececitamos anadir a estructura de HTTPRequest una prueba si request
	tiene CGI notificación (f.e. bool is_cgi) para no empesar un ciclo de
	CGI sin necesidad


*/

CGI::CGI(const HTTPRequest& req) 
{
	reset();

	env["CONTENT_LENGTH"] = findHeaderIgnoreCase(req, "content-length");
	env["CONTENT_TYPE"] = findHeaderIgnoreCase(req, "content-type");
	env["GATEWAY_INTERFACE"] = "CGI/1.1"; // por defecto
	env["QUERY_STRING"] = req.uri.query;
	env["PATH_INFO"] = req.uri.path;
	env["REQUEST_METHOD"] = req.method;
	env["SCRIPT_NAME"] = req.uri.path;
	env["SERVER_NAME"] = "server"; // server_config
	env["SERVER_PORT"] = ; // TODO server_config
	env["SERVER_PROTOCOL"] = req.protocol;
	env["SERVER_SOFTWARE"] = "webserver"; // este variable es obligatorio, pero no influye a script call

}

CGI::~CGI() {}

void CGI::reset()
{
	env.clear();
}

std::string CGI::findHeaderIgnoreCase(const HTTPRequest& req_headers, const std::string& headerToFind)
{
	std::string res = headerToFind;
	std::transform(result.begin(), result.end(), result.begin(), (int(*)(int)) std::tolower);

	// for (std::map<std::string, std::string>::const_iterator it = req_headers.fields.begin(); it != req_headers.fields.end(); ++it)
	for (HTTPRequest::const_iterator it = headers.fields.begin(); it != headers.fields.end(); ++it)
    {
        if (strcasecmp(it->first.c_str(), key.c_str()) == 0)
            return (it->second);
    }
    return ("");	
}
