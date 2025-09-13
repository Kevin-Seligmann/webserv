#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include "Logger.hpp"
#include "Utils.hpp"

/*
    This class is a persistent member of other classes meant to manage multiple instances 
    of opening and requesting file information, 
    hiding linux implementation details to provide a more comfortable interface.
    And providing a weak form of RAII.
*/

class File 
{
public:
    enum filetype {NONE, REGULAR, DIRECTORY};
    enum descriptor_status {EXISTS, OK, NOTFOUND, NOPERM, BADFILENAME, RAREFILE, ERROR};
    enum creation_status {S_NONE, NEW, OLD};

    File(std::string const & path, int mode);
    File();
    ~File();

    void open(std::string const & path, int mode, int f = 0);
    void close();

    off_t size() const;
    time_t last_modified() const;
    descriptor_status get_status() const;

    struct dirent * dir_next();

    int fd;
    enum filetype filetype;
    enum creation_status creation_status;
    
private:
    struct stat _statbuf;
    DIR * _dir;

    void handle_error();
    void destroy_current();
    void open_file(std::string const & src, int mode, int f = 0);
    enum descriptor_status status;

    File(File const &);
    File & operator=(File const &);
};
