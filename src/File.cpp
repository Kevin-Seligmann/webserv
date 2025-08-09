#include "File.hpp"

File::File()
{
    fd = -1;
    _dir = NULL;
    filetype = NONE;
}

File::File(std::string const & path, int mode)
:fd(-1), filetype(NONE), _dir(NULL)
{
    open_file(path, mode, 0);
}

void File::open(std::string const & path, int mode, int f = 0)
{
    destroy_current();
    open_file(path, mode, f);
}

void File::close(){destroy_current();}

File::~File()
{
    destroy_current();
}

off_t File::size() const
{
    if (filetype != REGULAR)
        CODE_ERR(std::string("Unexpectedly asking size of a non-regular file"));
    return _statbuf.st_size;
}

time_t File::last_modified() const
{
    if (filetype == NONE)
        CODE_ERR(std::string("Unexpectedly asking last-modified of an invalid or unintiialized state"));
    return _statbuf.st_mtime;
}

File::descriptor_status File::get_status() const
{
    // if (filetype == NONE)
    //     CODE_ERR(std::string("Unexpectedly asking status of an invalid or unintiialized state"));
    return status;
}

struct dirent * File::dir_next()
{
    return readdir(_dir);
}

void File::destroy_current()
{
    if (fd > -1)
    {
        ::close(fd);
        fd = -1;
    }
    if (_dir)
    {
        closedir(_dir);
        _dir = NULL;
    }
    filetype = NONE;
}

void File::open_file(std::string const & path, int mode, int f)
{
    status = OK;
    fd = ::open(path.c_str(), mode, f);
    if (fd == -1)
    {
        handle_error();
        return ;
    }
    if (fstat(fd, &_statbuf) == -1)
    {
        handle_error();
        return ;
    }
    if (S_ISREG(_statbuf.st_mode) || S_ISLNK(_statbuf.st_mode))
    {
        filetype = REGULAR;
    }
    else if (S_ISDIR(_statbuf.st_mode))
    {
        filetype = DIRECTORY;
        _dir = fdopendir(fd);
        if (!_dir)
        {
            handle_error();
            return ;
        }
    }
    else
    {
        status = RAREFILE;
        return ;
    }
}
    
void File::handle_error()
{
    switch (errno)
    {
        case ENOENT: status = NOTFOUND; return;
        case EPERM:
        case EROFS:
        case EFAULT:
        case EACCES: status = NOPERM; return ;
        case EINVAL: status = BADFILENAME; return ;
        default: status = ERROR; return ;
    }
    destroy_current();
}

