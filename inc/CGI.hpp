/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/07 16:02:03 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/23 17:08:41 by irozhkov         ###   ########.fr       */
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

# include "CGIEnv.hpp"
# include "CGIMap.hpp"
# include "HTTPRequest.hpp"

class CGI
{
	private:
		CGIEnv								_env;
		CGIMap                              _map;

		std::string methodToString(HTTPMethod method) const;

	public:

//	std::string 						req_body;

//	int req_pipe[2];
//	int cgi_pipe[2];
//	pid_t pid;

//	std::string resp;

	CGI(const HTTPRequest& req, const ParsedServer& server);
	~CGI();

	void buildEnv(const HTTPRequest &req, const ParsedServer& server);
	CGIEnv& getEnv();

	void reset();
//	std::string findHeaderIgnoreCase(const HTTPRequest& req_headers, const std::string& headerToFind);
//	void readCGIOut(int fd);
//	bool setPipeFlags(int fd);
//	void runCGI();
};

#endif
