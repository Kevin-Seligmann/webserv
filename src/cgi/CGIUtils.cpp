#include "CGIUtils.hpp"

namespace CGIUtils
{

std::string methodToString(HTTPMethod method)
{
	switch (method)
	{
		case GET: return "GET";
		case POST: return "POST";
		case DELETE: return "DELETE";
		default: return "UNKNOWN";
	}
}

std::string intToString(int value)
{
	std::stringstream ss;
	ss << value;
	return (ss.str());
}

std::string socketToPort(int socket_fd)
{
	struct sockaddr_in addr;

	socklen_t len = sizeof(addr);
	if (getsockname(socket_fd, (struct sockaddr*)&addr, &len) == -1) 
	{
		exit(1); // TODO need to find out the way to keep it safe
	}

	return (intToString(ntohs(addr.sin_port)));
}

std::string getExtension(const std::string& path)
{
	size_t dot = path.find_last_of('.');
	if (dot == std::string::npos)
		return ("");

	return (path.substr(dot));
}

char** reqToEnvp(const std::map<std::string, std::string>& src)
{
	std::vector<std::string> temp;
	for (std::map<std::string, std::string>::const_iterator it = src.begin(); it != src.end(); ++it)
		temp.push_back(it->first + "=" + it->second);

	char** envp = new char*[temp.size() + 1];
	for (size_t i = 0; i < temp.size(); ++i)
		envp[i] = strdup(temp[i].c_str());  
	envp[temp.size()] = NULL;

	return (envp);
}

char** reqToArgs(const std::map<std::string, std::string>& src, const std::map<std::string, std::string>& map)
{
	std::string extencion = getExtension(src.at("PATH_INFO"));

	std::string interpret;
    if (map.find(extencion) != map.end())
    {
        interpret = map.at(extencion);
    }
    else { interpret = ""; }

	std::string root = "/home/irozhkov/ws_repo"; // TODO change to root from serverconfig
	std::string scriptPath = root + src.at("PATH_INFO");

	char** args = new char*[3];
    args[0] = strdup(interpret.c_str());
	args[1] = strdup(scriptPath.c_str());
	args[2] = NULL;

	return (args);
}

void freeEnvp(char** envp)
{
    if (!envp)
		return ;

	for (size_t i = 0; envp[i] != NULL; ++i)
        free(envp[i]);
	delete[] envp;
}

}

