#include <fstream>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <iomanip>
#include <sys/stat.h>
#include <iostream>
#include "MediaType.hpp"
#include "RequestManager.hpp"
#include "Logger.hpp"
#include "HTTPRequest.hpp"
#include "RequestValidator.hpp"
#include "ElementParser.hpp"
#include "RequestParser.hpp"

void create_test_suite(const std::string & path, std::vector<std::string> & tests)
{
    struct stat st;
    stat(path.c_str(), &st);
    if (!S_ISDIR(st.st_mode))
    {
        tests.push_back(path);
        return ;
    }

    DIR *dir;
    struct dirent *ent;

    dir = opendir(path.c_str());
    while ((ent = readdir(dir)))
    {
        if ((ent->d_name)[0] == '.')
            continue ;
        
        std::string name = ent->d_name;
        std::string full_path = path + "/" + name;
        struct stat st;
    
        stat(full_path.c_str(), &st);
        if (S_ISDIR(st.st_mode))
            create_test_suite(full_path, tests);
        else
            tests.push_back(full_path);
    }
    closedir(dir);
}

void print_request(const char * c, size_t bytes)
{
    int max = 100, i;

    for (i = 0; i < bytes && i < max; i ++)
    {
        if (isprint(c[i]))
            std::cout << c[i];
        else 
        {
            switch (c[i])
            {
                case '\n': std::cout << "\\n"; break;
                case '\t': std::cout << "\\t"; break;
                case '\r': std::cout << "\\r"; break;
                case '\a': std::cout << "\\a"; break;
                case '\b': std::cout << "\\b"; break;
                case '\v': std::cout << "\\v"; break;
                case '\f': std::cout << "\\f"; break;
                case '\0': std::cout << "\\0"; break;
                default: std::cout << "\\" << std::hex << std::setw(2) << std::setfill('0') << c[i] << std::dec;
            }        
        }
    }
    if (i == max)
        std::cout << "...";
}

void run_request_parsing_test(std::string filename)
{
    std::string msg;
    char buffer[10000];
    char c;
    int f = open(filename.c_str(), O_RDONLY);
    int n = 0;

    while (read(f, &c, 1))
    {
        if (c == '\n')
            break ;
        else
            buffer[n] = c;
        n ++;
    }
    buffer[n] = 0;
    int num_requests = atoi(buffer);

    n = 0;
    while (read(f, &c, 1))
    {
        if (c == '\n')
            break;
        else
            buffer[n] = c;
        n++;
    }
    buffer[n] = 0;
    int expected_status = atoi(buffer);

    size_t start = lseek(f, 0, SEEK_CUR);
    ssize_t bytes = read(f, buffer, sizeof(buffer) - 1);
    std::string::size_type pos = filename.find_last_of("/\\");
    std::string basename = (pos == std::string::npos) ? filename : filename.substr(pos + 1);
    std::cout << "Testing... " << Logger::YELLOW  << basename << Logger::RESET << std::endl;
    print_request(buffer, bytes);
    std::cout << std::endl;
    lseek(f, start, SEEK_SET);
    
    HTTPRequest request;
    RequestManager manager(request, SysBufferFactory::SYSBUFF_FILE, f);


    int requests_done = 0;
    while (requests_done < num_requests)
    {
        manager.process();
        if  (manager.gerError().status() != OK)
            break ;
        if (manager.request_done())
        {
            requests_done ++;
            msg = request.to_string();
            // if (msg.size() > 100)
            //     msg = msg.substr(0, 100) + "...\n";
           //  std::cout << msg;
            manager.new_request();
        }
    }

    if (manager.gerError().status() == expected_status)
        std::cout << Logger::GREEN << "OK. " << Logger::RESET;
    else
        std::cout << Logger::RED << "FAIL. " << Logger::RESET;

    msg = manager.gerError().msg();
    if (msg.size() > 100)
        msg = msg.substr(0, 100) + "...";

    std::cout 
    << "File: " << basename 
    << ". Expected " << Logger::BLUE << status::status_to_text((Status) expected_status) << Logger::RESET 
    << " Got " << Logger::BLUE << status::status_to_text((Status) manager.gerError().status() ) 
    << ": " << Logger::RESET << msg << "\n" << std::endl;
}

void test_request_parsing(std::string src)
{

	MediaType::load_types();
    std::vector<std::string> tests;
    create_test_suite(src, tests);

    for (std::vector<std::string>::iterator it = tests.begin(); it != tests.end(); it ++)
       run_request_parsing_test(*it);
}

void test_mime_loading()
{
	MediaType::load_types();
	for (t_mime_conf::iterator it = MediaType::ACCEPTED_TYPES.begin(); it != MediaType::ACCEPTED_TYPES.end(); it ++)
	{
		std::cout << it->first;
        for (std::vector<std::string>::iterator s = it->second.begin(); s != it->second.end(); s ++)
            std::cout << "\"" << *s << "\"" << ", ";
        std::cout << std::endl;
	}
} 

int main(int argc, char ** argv)
{
    std::string src;

    if (argc == 2)
        src = argv[1];
    else
        src = "testfiles";
    test_request_parsing(src);
    // test_mime_loading();
}
