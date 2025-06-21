/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/07 16:02:03 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/21 19:33:33 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# pragma once

# include <algorithm>
# include <fcntl.h>
# include <map>
# include <string>
# include <strings.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
# include <vector>

# include "HTTPRequest.hpp"

struct CGI
{
	std::map<std::string, std::string> env;
	std::map<std::string, std::string> cgi_map;
	std::string req_body;

	int req_pipe[2];
	int cgi_pipe[2];
	pid_t pid;

	std::string resp;

	CGI(const HTTPRequest& req);
	~CGI();

	void reset();
	std::string findHeaderIgnoreCase(const HTTPRequest& req_headers, const std::string& headerToFind);
	bool setPipeFlags(int fd);
	void runCGI();
	void readCGIOut(int fd);
};

#endif
