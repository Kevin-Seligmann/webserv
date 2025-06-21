/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/07 16:14:39 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/21 19:38:54 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"
#include "CGIUtils.hpp"

/*
	Nececitamos anadir a estructura de HTTPRequest una prueba si request
	tiene CGI notificación (f.e. bool is_cgi) para no empesar un ciclo de
	CGI sin necesidad


*/

CGI::CGI(const HTTPRequest& req) 
{
	reset();

	cgi_map[".py"] = "/usr/bin/python3";
	cgi_map[".php"] = "/usr/bin/php-cgi";

	env["CONTENT_LENGTH"] = findHeaderIgnoreCase(req, "content-length");
	env["CONTENT_TYPE"] = findHeaderIgnoreCase(req, "content-type");
	env["GATEWAY_INTERFACE"] = "CGI/1.1"; // por defecto
	env["QUERY_STRING"] = req.uri.query;
	env["PATH_INFO"] = req.uri.path;
	env["REQUEST_METHOD"] = CGIUtils::methodToString(req.method);
	env["SCRIPT_NAME"] = req.uri.path;
	env["SERVER_NAME"] = "server"; // server_config
	env["SERVER_PORT"] = "8080"; // TODO server_config
	env["SERVER_PROTOCOL"] = req.protocol;
	env["SERVER_SOFTWARE"] = "webserver"; // este variable es obligatorio, pero no influye a script call

	req_body = req.body.raw;
}

CGI::~CGI() {}

void CGI::reset()
{
	env.clear();
}

std::string CGI::findHeaderIgnoreCase(const HTTPRequest& req_headers, const std::string& headerToFind)
{
	std::string res = headerToFind;
	std::transform(res.begin(), res.end(), res.begin(), (int(*)(int)) std::tolower);

	for (std::map<std::string, std::vector<std::string> >::const_iterator it = req_headers.headers.fields.begin();
         it != req_headers.headers.fields.end(); ++it)
    {
        if (strcasecmp(it->first.c_str(), headerToFind.c_str()) == 0)
        {
            if (!it->second.empty())
                return it->second.front();
        }
    }
    return ("");
}

bool CGI::setPipeFlags(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        return (false);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return (false);

    flags = fcntl(fd, F_GETFD);
    if (flags == -1)
        return (false);
    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
        return (false);
	
    return (true);
}

void CGI::runCGI()
{
	if (env["PATH_INFO"].find("/cgi-bin/") == std::string::npos)
		return ;

	if (pipe(req_pipe) == -1 || pipe(cgi_pipe) == -1)
		exit(1);

/*	if (!setPipeFlags(req_pipe[0]) || !setPipeFlags(req_pipe[1]) ||
		!setPipeFlags(cgi_pipe[0]) || !setPipeFlags(cgi_pipe[1]))
		exit(1);*/


	std::cout << "----------------->" << req_body << std::endl;

	ssize_t n;

	n = write(req_pipe[1], req_body.c_str(), req_body.size());
	if (n == -1)
	{
		perror("write failed to CGI stdin");
	}

	pid = fork();

	if (pid == -1)
		exit(1);

	if (pid == 0)
	{
		if (dup2(req_pipe[0], STDIN_FILENO) == -1 ||
			dup2(cgi_pipe[1], STDOUT_FILENO) == -1)
			exit(1);

		if (close(req_pipe[0]) || close(req_pipe[1]) || 
			close(cgi_pipe[0]) || close(cgi_pipe[1]))
			exit(1);

		char** args = CGIUtils::reqToArgs(env, cgi_map);
		char** envp = CGIUtils::reqToEnvp(env);

		execve(args[0], args, envp);

		exit(1);
	}

	close(req_pipe[0]);
	close(req_pipe[1]);
	close(cgi_pipe[1]);

	if (cgi_pipe[0] < 0)
		exit(1);

	if (fcntl(cgi_pipe[0], F_GETFD) == -1)
		exit(1);

	readCGIOut(cgi_pipe[0]);

	std::cout << ">>>>>>>>>> " << resp << std::endl;

    close(cgi_pipe[0]);

    int status;
    waitpid(pid, &status, 0);

//	int cgi_fd = cgi_pipe[0];

//	CGIUtils::freeEnvp(envp);
//	CGIUtils::freeEnvp(args);
}

void CGI::readCGIOut(int fd)
{
	ssize_t n;
	char buf[1024];

	while ((n = read(fd, buf, sizeof(buf) - 1)) > 0)
	{
		buf[n] = '\0';
		resp += buf;
	}

//	close(fd);
}
