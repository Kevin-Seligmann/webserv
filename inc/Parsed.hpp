/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsed.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:51:01 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/02 15:26:11 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSED_HPP
# define PARSED_HPP

# pragma once

# include <fstream>
# include <iostream>
# include <map>
# include <sstream>
# include <stdexcept>
# include <string>
# include <vector>

struct ParsedLocations
{
    std::string path;
    std::string root;
    std::string index;
    std::vector<std::string> allow_methods;
    bool autoindex;
    std::string return_path;
    bool allow_upload;
    std::string upload_dir;
    std::map<std::string, std::string> cgi;

    ParsedLocations()
        : autoindex(false), allow_upload(false) {}
};

struct ParsedListen
{
    std::string ip;
    int port;
    bool is_default;

    ParsedListen()
        : ip("0.0.0.0"), port(8080), is_default(false) {}

    std::string to_string() const
    {
        std::ostringstream oss;
        oss << ip << ":" << port;
        if (is_default) oss << " [default_server]";
        return oss.str();
    }
};

struct ParsedServer
{
    std::vector<ParsedListen> listens;
    std::vector<std::string> server_names;
    std::string root;
    std::map<int, std::string> error_pages;
    std::vector<std::string> allow_methods;
    bool autoindex;
    std::string client_max_body_size;
    std::map<std::string, ParsedLocations> locations;

    ParsedServer() : autoindex(false) {}
};

std::vector<std::string> tokenize(const std::string& content);
size_t expect(const std::vector<std::string>& tokens, size_t i, const std::string& expected);
int to_int(const std::string& s);
ParsedListen parse_listen(const std::vector<std::string>& tokens);
ParsedServer parseServer(const std::vector<std::string>& tokens, size_t& i);
std::vector<ParsedServer> parseConfig(const std::vector<std::string>& tokens);

#endif
