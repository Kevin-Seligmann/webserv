#include "../inc/ServersManager.hpp"
#include <iostream>
#include <sstream>

// para dev de init solamente
ParsedServers& fakeConfig(ParsedServers& ps) {

    // dos servers con igual host:ip y diferente nombre (name1 y name2)
    for (int i = 1; i < 3; ++i) {
        std::vector<std::string> methods;
        methods.push_back("GET");
        methods.push_back("POST");
        ParsedLocations aux_loc1(
            "/",
            methods,
            "",
            false,
            "",
            "",
            "",
            ""
        );
        ParsedLocations aux_loc2(
            "/",
            methods,
            "",
            false,
            "",
            "",
            "",
            ""
        );

        std::ostringstream oss;
        oss << "name" << i;
        ParsedServer aux_server(
            "127.0.0.1",
            80,
            oss.str(),
            "",
            std::vector<std::string>(),
            std::map<int, std::string>(),
            std::vector<ParsedLocations>()
        );
        aux_server.locations.push_back(aux_loc1);
        aux_server.locations.push_back(aux_loc2);

        ps.push_back(aux_server);
    }

    // dos servers con igual host:ip sin nombre y con distintos metodos
    for (int i = 1; i < 3; ++i) {
        std::vector<std::string> methods1;
        methods1.push_back("GET");
        ParsedLocations aux_loc3(
            "/",
            methods1,
            "",
            false,
            "",
            "",
            "",
            ""
        );

        std::vector<std::string> methods2;
        methods2.push_back("POST");
        ParsedLocations aux_loc4(
            "/",
            methods2,
            "",
            false,
            "",
            "",
            "",
            ""
        );

        ParsedServer aux_server(
            "128.0.0.1",
            80,
            "",
            "",
            std::vector<std::string>(),
            std::map<int, std::string>(),
            std::vector<ParsedLocations>()
        );
        aux_server.locations.push_back(aux_loc3);
        aux_server.locations.push_back(aux_loc4);

        ps.push_back(aux_server);
    }

    // un server solo
    {
        ParsedLocations aux_loc(
            "",
            std::vector<std::string>(),
            "",
            false,
            "",
            "",
            "",
            ""
        );
        ParsedServer aux_server(
            "129.0.0.1",
            0,
            "",
            "",
            std::vector<std::string>(),
            std::map<int, std::string>(),
            std::vector<ParsedLocations>()
        );
        aux_server.locations.push_back(aux_loc);
        ps.push_back(aux_server);
    }

    return ps;
}

bool serversInit(ServersManager& sm, const ParsedServers& ps) {

    for (size_t i = 0; i < ps.size(); ++i) {
        Servers aux_server(ps[i]);
        HostPort hp(ps[i].host, ps[i].port);
        sm.addServer(hp, aux_server); // maneja la lógica de default_server
    }
    std::cout << "Success: servers initialized." << std::endl;
    return true;
}
// fin dev de init