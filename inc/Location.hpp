#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <vector>
#include <map>

class Location {
private:
	std::string								_path;
	std::vector<std::string>				_methods;
	std::string								_root;
	std::string								_index;
	bool									_directory_listing;
	std::string                             _redirect;
	std::string 							_cgi_extension;
	std::string                             _upload_path;

public:
	Location();
	Location(const Location& parsed_location);
	~Location();

};

#endif