#include <fstream>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include "test.hpp"
#include "RequestManager.hpp"
#include "Logger.hpp"
#include "HTTPRequest.hpp"
#include "RequestValidator.hpp"
#include "ElementParser.hpp"
#include "RequestParser.hpp"

int n = 1;

std::vector<std::string> create_test_suite()
{
    DIR *dir;
    struct dirent *ent;
    std::vector<std::string> tests;

    if ((dir = opendir ("./testfiles")) != NULL)
    {
        while ((ent = readdir (dir)) != NULL)
            if ((ent->d_name)[0] != '.')
                tests.push_back("./testfiles/" + std::string(ent->d_name));
        closedir (dir);
    } 
    else
        std::cout << "Opening files failed" << std::endl;
    return tests;
}

void run_request_parsing_test(std::string filename)
{
    char s[4];
    char c;
    int f = open(filename.c_str(), O_RDONLY);
    int n = 0;

    while (read(f, &c, 1))
    {
        if (c == '\n')
            break ;
        else
            s[n] = c;
        n ++;
    }
    s[n] = 0;
    int num_requests = atoi(s);

    n = 0;
    while (read(f, &c, 1))
    {
        if (c == '\n')
            break;
        else
            s[n] = c;
        n++;
    }
    s[n] = 0;
    int expected_status = atoi(s);

 
    HTTPRequest request;
    RequestManager manager(request, SysBufferFactory::SYSBUFF_FILE, f);

    std::cout << "Testing... " << filename << std::endl;

    int requests_done = 0;
    while (requests_done < num_requests)
    {
        manager.process();
        if  (manager.gerError().status() != OK)
            break ;
        if (manager.request_done())
        {
            requests_done ++;
            manager.new_request();
        }
    }

    if (manager.gerError().status() == expected_status)
        std::cout << Logger::GREEN << "OK. " << Logger::RESET;
    else
        std::cout << Logger::RED << "FAIL. " << Logger::RESET;

    std::cout << filename << ", got " << manager.gerError().to_string() << " --- Expected " << status::status_to_text((Status) expected_status) << "\n" << std::endl;
}

void test_request_parsing()
{
    std::vector<std::string> tests = create_test_suite();

    for (std::vector<std::string>::iterator it = tests.begin(); it != tests.end(); it ++)
       run_request_parsing_test(*it);
}

int main()
{
    test_request_parsing();
}
