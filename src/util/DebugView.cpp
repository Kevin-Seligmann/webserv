/*
#include "DebugView.hpp"
#include "Utils.hpp"

static void printLine(const std::string &s, size_t width) {
    std::string line = s;
    if (line.size() > width) line = line.substr(0, width);
    if (line.size() < width) line += std::string(width - line.size(), ' ');
    Logger &log = Logger::getInstance();
    log.info("| " + line + " |");
}

void DebugView::printBox(const std::string &title, const std::vector<std::string> &lines) {
    Logger &log = Logger::getInstance();
    const size_t width = 66; // interior width
    const std::string top = "+" + std::string(width + 2, '=') + "+"; // +==...==+
    log.info(top);
    std::string titleLine = "[ " + title + " ]";
    if (titleLine.size() > width) titleLine = titleLine.substr(0, width);
    if (titleLine.size() < width) titleLine += std::string(width - titleLine.size(), ' ');
    log.info("| " + titleLine + " |");
    log.info(top);
    for (size_t i = 0; i < lines.size(); ++i) {
        printLine(lines[i], width);
    }
    log.info(top);
}

void DebugView::printConfigSummary(const ParsedServers &configs) {
    std::cout << "PRINT CONFIG SUMMARY" << std::endl;
    std::vector<std::string> lines;
    lines.push_back("servers: " + wss::i_to_dec(configs.size()));
    for (size_t i = 0; i < configs.size(); ++i) {
        const ParsedServer &srv = configs[i];
        // Header line per server
        std::string header = "srv " + wss::i_to_dec(i + 1) + " | listens: " + wss::i_to_dec(srv.listens.size());
        lines.push_back(header);

        // Each listen
        for (size_t j = 0; j < srv.listens.size(); ++j) {
            const Listen &l = srv.listens[j];
            std::string is_def = l.is_default ? " default" : "";
            lines.push_back("  - listen " + l.host + ":" + wss::i_to_dec(l.port) + is_def);
        }

        // Names (compact)
        if (!srv.server_names.empty()) {
            std::string names = "  names: ";
            for (size_t k = 0; k < srv.server_names.size(); ++k) {
                names += srv.server_names[k];
                if (k + 1 < srv.server_names.size()) names += ",";
            }
            lines.push_back(names);
        } else {
            lines.push_back("  names: -");
        }

        // Root, index count, methods count, locations count
        lines.push_back("  root: " + (srv.root.empty() ? std::string("-") : srv.root));
        lines.push_back("  index: " + wss::i_to_dec(srv.index_files.size()));

        lines.push_back("  methods: " + wss::i_to_dec(srv.allow_methods.size()));
        std::vector<std::string>::const_iterator m_it = srv.allow_methods.begin();
        for (;m_it != srv.allow_methods.end(); ++m_it) {
            lines.push_back("      allowed method: '" + *m_it + "'");
        }

        lines.push_back("  locations: " + wss::i_to_dec(srv.locations.size()));
        std::map<std::string, Location>::const_iterator l_it = srv.locations.begin();
        for (;l_it != srv.locations.end(); ++l_it) {
            lines.push_back("      location path value: '" + l_it->first + "'");
        }
    }

    printBox("CONFIG SUMMARY", lines);
}
*/