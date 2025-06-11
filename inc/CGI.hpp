/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/07 16:02:03 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/07 20:33:54 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# pragma once

# include <algorithm>
# include <map>
# include <string>
# include <strings.h>

# include "HTTPRequest.hpp"

struct CGI
{
	std::map<std::string, std::string> env;

	CGI(const HTTPRequest& req);
	~CGI();

	void reset();
	std::string findHeaderIgnoreCase(const HTTPRequest& req_headers, const std::string& headerToFind);
};

#endif
