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

	if (interpreter.empty()) interpreter = script_filename;

	_size = 3;
	_args = new char*[_size];

	_args[0] = strdup(interpreter.c_str());
	_args[1] = strdup(script_filename.c_str());
	_args[2] = NULL;
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
