#include "../inc/Parsed.hpp"

ParsedLocations::ParsedLocations(
    const std::string& path,
    const std::vector<std::string>& validMethods,
    const std::string& root,
    bool autoindex,
    const std::string& cgiExtension,
    const std::string& cgiPath,
    const std::string& redir,
    const std::string& uploadDir
)
  : path(path)
  , validMethods(validMethods)
  , root(root), autoindex(autoindex)
  , cgiExtension(cgiExtension)
  , cgiPath(cgiPath)
  , redir(redir)
  , uploadDir(uploadDir)
{}

ParsedLocations::~ParsedLocations() {}

ParsedServer::ParsedServer(
    const std::string& host,
    int port,
    const std::string& serverName,
    const std::string& root,
    const std::vector<std::string>& indexFiles,
    const std::map<int, std::string>& errorPages,
    const std::vector<ParsedLocations>& locations
)
  : host(host)
  , port(port)
  , serverName(serverName)
  , root(root)
  , indexFiles(indexFiles)
  , errorPages(errorPages)
  , locations(locations)
{}

ParsedServer::~ParsedServer() {}