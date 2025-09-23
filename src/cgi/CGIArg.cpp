/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIArg.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvisca-g <mvisca-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/06 14:23:15 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/22 17:05:52 by mvisca-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIArg.hpp"
#include <unistd.h>

CGIArg::CGIArg(const CGIEnv& env) : _args(NULL), _size(0)
{
	std::string script = env.getCGIEnvValue("SCRIPT_NAME");

	std::string script_filename = env.getCGIEnvValue("SCRIPT_FILENAME");
	
	std::string interpreter = CGIInterpreter::findInterpreterForScript(script);
	
	// TEST
	std::cerr << "CGIArg DEBUG: SCRIPT_NAME=[" << script << "]" << std::endl;
    std::cerr << "CGIArg DEBUG: SCRIPT_FILENAME=[" << env.getCGIEnvValue("SCRIPT_FILENAME") << "]" << std::endl;
    std::cerr << "CGIArg DEBUG: interpreter=[" << interpreter << "]" << std::endl;
	// .

	if (interpreter.empty())
	{
		if (access(script_filename.c_str(), X_OK) == 0) 
		{
			interpreter = script_filename;
			// TEST
			std::cerr << "CGIArg DEBUG: Using script as executable" << std::endl;	
			// .
		}
		else
		{
			std::string ext = script.substr(script.find_last_of('.'));
			if (ext == ".bla")
			{
				interpreter = "/";
				// TEST
				std::cerr << "CGIArg DEBUG: Using default interpreter for .bla -> '/'" << std::endl;
				// .
			}
			else
			{
				interpreter = script_filename;
			}
		}
	}

	if (interpreter == script_filename)
	{
		_size = 2;
        _args = new char*[_size];
        _args[0] = strdup(interpreter.c_str());
        _args[1] = NULL;
	}
	else{
		_size = 3;
		_args = new char*[_size];
		
		_args[0] = strdup(interpreter.c_str());
		_args[1] = strdup(script_filename.c_str());
		_args[2] = NULL;
	}


	// TEST
	std::cerr << "CGIArg DEBUG: argv[0]=" << _args[0]
              << " argv[1]=" << (_size > 2 ? _args[1] : "(none)") << std::endl;
	// .
}


CGIArg::~CGIArg()
{
	if (_args)
	{
		for (size_t i = 0; i < _size; ++i)
			free(_args[i]);
		delete[] _args;
	}
}

char** CGIArg::getArgs() const
{
	return (_args);
}
