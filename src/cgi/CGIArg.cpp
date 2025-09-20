/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIArg.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/06 14:23:15 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/18 12:11:13 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIArg.hpp"

CGIArg::CGIArg(const CGIEnv& env) : _args(NULL), _size(0)
{
	std::string script = env.getCGIEnvValue("SCRIPT_NAME");
	std::string interpreter = CGIInterpreter::findInterpreterForScript(script);

	if (interpreter.empty()) interpreter = script;

	_size = 3;
	_args = new char*[_size];

	_args[0] = strdup(interpreter.c_str());
	_args[1] = strdup(script.c_str());
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
