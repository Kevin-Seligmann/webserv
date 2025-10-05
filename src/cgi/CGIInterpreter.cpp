/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIInterpreter.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvisca-g <mvisca-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 11:11:22 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/29 17:40:54 by mvisca-g         ###   ########.fr       */
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

			// Validate that the interpreter is accessible
			if (access(interpreter.c_str(), F_OK) != 0)
			{
				throw std::runtime_error("Couldn't find: " + interpreter);
			}
			
			if (access(interpreter.c_str(), X_OK) != 0)
			{
				throw std::runtime_error("Faild to execute: " + interpreter);
			}

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
			{
				return (it->interpreter);
			}
		}
	}
	return ("");
}

std::string CGIInterpreter::findInterpreterForScript(const std::string& script)
{
	for (t_cgi_conf::const_iterator it = ACCEPTED_EXT.begin(); it != ACCEPTED_EXT.end(); ++it)
	{
		for (std::vector<std::string>::const_iterator s = it->extensions.begin();
			 s != it->extensions.end(); ++s)
		{
			if (script.size() >= s->size() &&
				script.compare(script.size() - s->size(), s->size(), *s) == 0)
			{
				return it->interpreter;
			}
		}
	}
	return "";
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
