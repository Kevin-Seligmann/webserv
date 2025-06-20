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

    s[3] = 0;
    int expected_status = atoi(s);
 
    HTTPRequest request;
    RequestManager manager(request, SysBufferFactory::SYSBUFF_FILE, f);

    while (1)
    {
        manager.process();
        if  (manager.gerError().status() != OK)
        {
            std::cout << "Error" << std::endl;
            break ;
        }
        if (manager.request_done())
        {
            manager.new_request();
            std::cout << "Did request" << std::endl;
        }
        // if (manager.gerError().status() != OK || manager.request_done())
        //     break ;
    }

    if (manager.gerError().status() == expected_status)
        std::cout << Logger::GREEN << "OK. " << Logger::RESET;
    else
        std::cout << Logger::RED << "FAIL. " << Logger::RESET;

    std::cout << filename << ", got " << manager.gerError().to_string() << " --- Expected " << expected_status << std::endl;

    // bool ok = true;
    // Logger & log = Logger::getInstance();
    // ErrorContainer error_container;
    // HTTPRequest request;
    // RequestValidator validator(request, error_container);
    // ElementParser element_parser(error_container);
    // RequestParser parser(request, error_container, element_parser, validator);

    // for (int i = 0; test.input_string[i]; i++)
    // {
    //     parser.append(((uint8_t *) test.input_string) + i, 1);
    //     parser.process();
    
    //     if (error_container.error())
    //         break ;
    //     if (parser.done())
    //     {
    //         parser.new_request();
    //         request.reset();
    //     }
    // }
    // test.obtained_status = error_container.error_status();

    // std::cout << Logger::BLUE << "Test. " << n << ": " << Logger::RESET;
    // if (test.obtained_status == test.expected_status)
    //     std::cout << Logger::GREEN << "OK. " << Logger::RESET << test.name << ". " << std::endl;
    // else 
    // {
    //     std::cout << Logger::RED << "ERROR. " << Logger::RESET << test.name << ". " << std::endl; 
    //     std::cout << "Expected: " << test.expected_status << ", found errors: " << std::endl;
    //     error_container.log_errors();
    // }

    // n++;
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


    // tests.push_back(
    //     RequestParseTest(
    //         "GOAT / HTTP/1.1\r\nHost: example.com\r\n\r\n",
    //         NOT_IMPLEMENTED,
    //         "unknown method"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GOAT / HTT.1\r\nHost: example.com\r\n\r\n",
    //         NOT_IMPLEMENTED,
    //         "unknown protocol"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GOAT /ª HTTP/1.1\r\nHost: example.com\r\n\r\n",
    //         BAD_REQUEST,
    //         "rare_uri_character"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET htt://example.com HTTP/1.1\r\nHost: example.com\r\n\r\n",
    //         NOT_IMPLEMENTED,
    //         "bad_uri_protocol"
    //     )
    // );


    // tests.push_back(
    //     RequestParseTest(
    //         "   \r\t\f\v   GET    \r\t\f\v  /   \r\t\f\v     HTTP/1.1    \r\t\f\v    \r\nHost: example.com\r\n\r\n",
    //         NO_STATUS,
    //         "leniant_parsing_not_allowed_whitespace"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\nHost: example.com\n\n",
    //         NO_STATUS,
    //         "leniant_parsing_no_cr"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET \2 / HTTP/1.1\nHost: example.com\n\n",
    //         BAD_REQUEST,
    //         "control_token_first_line"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET \2 / HTTP/1.1\n Host: example.com\n\n",
    //         BAD_REQUEST,
    //         "header_space_before_name"
    //     )
    // );

    // tests.push_back(
    // RequestParseTest(
    //     "GET\r\nHost: example.com\r\n\r\n",
    //     BAD_REQUEST,
    //     "missing_uri"
    // ));

    // tests.push_back(
    // RequestParseTest(
    //     "GET /very/long/path/with/many/segments/that/goes/on/and/on/file.html HTTP/1.1\r\nHost: example.com\r\n\r\n",
    //     NO_STATUS,
    //     "long_valid_path"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET /path%20with%20spaces HTTP/1.1\r\nHost: example.com\r\n\r\n",
    //         NO_STATUS,
    //         "url_encoded_path"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET /path?query=value&foo=bar#tea HTTP/1.1\r\nHost: example.com\r\n\r\n",
    //         NO_STATUS,
    //         "path_with_query_string_and_fragment"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET /../../../etc/passwd HTTP/1.1\r\nHost: example.com\r\n\r\n",
    //         NO_STATUS, 
    //         "path_traversal_attempt"
    //     )
    // );

    // // Header tests
    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\n\r\n",
    //         BAD_REQUEST, 
    //         "missing_required_host_header"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\nHost:\r\n\r\n",
    //         BAD_REQUEST,
    //         "empty_host_header_value"
    //     )
    // );


    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\nHost: example.com\r\nHost: example.org\r\n\r\n",
    //         BAD_REQUEST,
    //         "duplicate_host_header"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\nHost: example.com\r\nContent-Length: -5\r\n\r\n",
    //         BAD_REQUEST,
    //         "negative_content_length"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\nHost: example.com\r\nHeader-With-Colon:: value\r\n\r\n",
    //         NO_STATUS,
    //         "header_name_with_colon"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\nHost: example.com\r\nHeader\x01Name: value\r\n\r\n",
    //         BAD_REQUEST,
    //         "header_name_with_control_char"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\nHost: example.com\r\nHeader-Name: value\x7f\r\n\r\n",
    //         BAD_REQUEST,
    //         "header_value_with_control_char"
    //     )
    // );

    // // Malformed Request Line Tests
    // tests.push_back(
    //     RequestParseTest(
    //         "GET\r\nHost: example.com\r\n\r\n",
    //         BAD_REQUEST,
    //         "incomplete_request_line"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / \r\nHost: example.com\r\n\r\n",
    //         BAD_REQUEST,
    //         "missing_http_version"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET  /  HTTP/1.1\r\nHost: example.com\r\n\r\n",
    //         NO_STATUS,
    //         "multiple_spaces_in_request_line"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "\r\n\r\nGET / HTTP/1.1\r\nHost: example.com\r\n\r\n",
    //         BAD_REQUEST,
    //         "two_empty_lines"
    //     )
    // );

    // // Different HTTP Methods
    // tests.push_back(
    //     RequestParseTest(
    //         "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 0\r\n\r\n",
    //         NO_STATUS,
    //         "post_method"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "get / HTTP/1.1\r\nHost: example.com\r\n\r\n",
    //         NO_STATUS,
    //         "lowercase_method"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\n: invalid_header\r\n\r\n",
    //         BAD_REQUEST,
    //         "header_missing_name"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\nHost: example.com\r\nVery-Long-Header-Name-That-Exceeds-Normal-Limits-And-Goes-On-And-On: value\r\n\r\n",
    //         NO_STATUS,
    //         "very_long_header_name"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\nhost: example.com\r\n\r\n",
    //         NO_STATUS,
    //         "lowercase_header_name"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\nHOST: EXAMPLE.COM\r\n\r\n",
    //         NO_STATUS,
    //         "uppercase_header_name"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\nHost:   example.com   \r\n\r\n",
    //         NO_STATUS,
    //         "header_value_with_whitespace"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "GET / HTTP/1.1\r\nHost: example.com\r\n User-Agent: test\r\n\r\n",
    //         BAD_REQUEST,
    //         "header_line_starts_with_space"
    //     )
    // );

    // tests.push_back(
    //     RequestParseTest(
    //         "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 4\r\n\r\ntest",
    //         NO_STATUS,
    //         "post_with_body"
    //     )
    // );

    //     tests.push_back(
    //     RequestParseTest(
    //         "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 4\r\n\r\ntest\r\nPOST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 4\r\n\r\n",
    //         NO_STATUS,
    //         "two_valid_requests"
    //     )
    // );

    //         tests.push_back(
    //     RequestParseTest(
    //         "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 4\r\n\r\ntest\r\nPOSRT / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 4\r\n\r\n",
    //         BAD_REQUEST,
    //         "second_invalid_request"
    //     )
    // );

    //        tests.push_back(
    //     RequestParseTest(
    //         "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 4\r\n\r\ntest\r\nPOSRT / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 4\r\n\r\n",
    //         BAD_REQUEST,
    //         "path_too_long"
    //     )
    // );