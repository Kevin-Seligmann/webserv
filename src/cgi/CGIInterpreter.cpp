/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIInterpreter.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 11:11:22 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/15 13:43:46 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIInterpreter.hpp"

const std::string CGIInterpreter::INTERPRETERS_PATH = "./conf/cgi/cgi_extencions.csv";
t_cgi_conf CGIInterpreter::ACCEPTED_EXT;

CGIInterpreter::CGIInterpreter() {}
CGIInterpreter::CGIInterpreter(std::string interpreter, std::vector<std::string> extensions) : interpreter(interpreter), extensions(extensions) {}

void CGIInterpreter::load_interpreters()
{
	std::ifstream in;
	in.exceptions(std::ifstream::badbit);

	try 
	{
		in.open(INTERPRETERS_PATH.c_str());

		std::string line;
		std::getline(in, line); // Skip header line

		while (!in.eof() && std::getline(in, line))
		{
			if (line.empty()) // Skip if line is empty
				continue;

			std::stringstream ss(line);
			ss.exceptions(std::ifstream::badbit);

			std::string interpreter;
			std::getline(ss, interpreter, ',');
			wss::trim(interpreter);

			if (interpreter.empty()) // Skip if there is no interpreter
				continue;

			std::vector<std::string> extensions;
			std::string ext;
			while (!ss.eof() && std::getline(ss, ext, ','))
			{
				wss::trim(ext);
				extensions.push_back(ext);
			}

			ACCEPTED_EXT.push_back(CGIInterpreter(interpreter, extensions));
        }
    }
	catch (const std::ifstream::failure& e)
	{
		Logger::getInstance().warning("Error reading CGI INTERPRETERS configuration.");
		Logger::getInstance() << e.what();
		ACCEPTED_EXT.clear();
		return ;
    }
}

std::string CGIInterpreter::getInterpreterForExtension(const std::string& ext)
{
	for (t_cgi_conf::iterator it = ACCEPTED_EXT.begin(); it != ACCEPTED_EXT.end(); ++it)
	{
		for (std::vector<std::string>::iterator s = it->extensions.begin(); s != it->extensions.end(); ++s)
		{
			if (ext == *s)
				return (it->interpreter);
		}
	}
	return ("");
}

std::ostream& operator<<(std::ostream& os, CGIInterpreter const& cgi)
{
	os << "Interpreter: " << cgi.interpreter << ", Extensions: ";
	for (size_t i = 0; i < cgi.extensions.size(); ++i)
	{
		os << cgi.extensions[i];
		if (i + 1 != cgi.extensions.size()) os << ", ";
	}
	return (os);
}

std::ostream& operator<<(std::ostream& os, const t_cgi_conf& conf)
{
	for (size_t i = 0; i < conf.size(); ++i)
	{
		os << conf[i] << std::endl;
	}
	return (os);
}

void CGIInterpreter::reset()
{
	interpreter.clear();
	extensions.clear();
}
