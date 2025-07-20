#include "../inc/debug.hpp"
#include "../inc/Utils.hpp"
#include <iostream>
#include <sstream>

// FUNCIÓN PARA DEBUG DE PARSED CONFIG
void showParse(const ParsedServers& config) {
    std::cout << GREEN << "\n=== SERVERS CONFIG DEBUG ===" << RESET << std::endl;
    
    for (size_t i = 0; i < config.size(); ++i) {
        const ParsedServer& server = config[i];
        
        std::cout << BLUE << "\n--- Server " << (i + 1) << " ---" << RESET << std::endl;
        
        // Listen 
        std::cout << YELLOW << "Listen Directives:" << RESET << std::endl;
        if (server.listens.empty()) {
            std::cout << "  - " << RED << "Empty (will use default 0.0.0.0:8080)" << RESET << std::endl;
        } else {
            for (size_t j = 0; j < server.listens.size(); ++j) {
                const Listen& listen = server.listens[j];
                std::cout << "  - " << listen.to_string();
                if (listen.is_default) {
                    std::cout << GREEN << " [DEFAULT_SERVER]" << RESET;
                }
                std::cout << std::endl;
            }
        }
        
        // Server names
        std::cout << YELLOW << "Server Names:" << RESET << std::endl;
        if (server.server_names.empty()) {
            std::cout << "  - " << RED << "Empty" << RESET << std::endl;
        } else {
            for (size_t j = 0; j < server.server_names.size(); ++j) {
                std::cout << "  - " << server.server_names[j] << std::endl;
            }
        }
        
        // Root
        std::cout << YELLOW << "Root: " << RESET << server.root << std::endl;
        
        // Index files
        std::cout << YELLOW << "Index Files:" << RESET << std::endl;
        if (server.index_files.empty()) {
            std::cout << "  - " << RED << "Empty" << RESET << std::endl;
        } else {
            for (size_t j = 0; j < server.index_files.size(); ++j) {
                std::cout << "  - " << server.index_files[j] << std::endl;
            }
        }
        
        // Error pages
        std::cout << YELLOW << "Error Pages:" << RESET << std::endl;
        if (server.error_pages.empty()) {
            std::cout << "  - " << RED << "Empty" << RESET << std::endl;
        } else {
            for (std::map<int, std::string>::const_iterator it = server.error_pages.begin();
                 it != server.error_pages.end(); ++it) {
                std::cout << "  - " << it->first << " -> " << it->second << std::endl;
            }
        }
        
        // Allow methods
        std::cout << YELLOW << "Allow Methods:" << RESET << std::endl;
        if (server.allow_methods.empty()) {
            std::cout << "  - " << RED << "Empty" << RESET << std::endl;
        } else {
            for (size_t j = 0; j < server.allow_methods.size(); ++j) {
                std::cout << "  - " << server.allow_methods[j] << std::endl;
            }
        }
        
        // Autoindex
        std::cout << YELLOW << "Autoindex: " << RESET;
        if (server.autoindex) {
            std::cout << GREEN << "ON" << RESET;
        } else {
            std::cout << RED << "OFF" << RESET;
        }
        std::cout << std::endl;
        
        // Client max body size
        std::cout << YELLOW << "Client Max Body Size: " << RESET;
        if (server.client_max_body_size.empty()) {
            std::cout << RED << "Empty" << RESET;
        } else {
            std::cout << server.client_max_body_size;
        }
        std::cout << std::endl;
        
        // Locations
        std::cout << YELLOW << "Locations:" << RESET << std::endl;
        if (server.locations.empty()) {
            std::cout << "  - " << RED << "Empty" << RESET << std::endl;
        } else {
            for (std::map<std::string, Locations>::const_iterator it = server.locations.begin();
                 it != server.locations.end(); ++it) {
                const std::string& path = it->first;
                const Locations& loc = it->second;
                
                std::cout << "  - Path: " << YELLOW << path << RESET << std::endl;
                
                std::cout << "    Root: ";
                if (loc.root.empty()) {
                    std::cout << RED << "Empty" << RESET;
                } else {
                    std::cout << loc.root;
                }
                std::cout << std::endl;
                
                std::cout << "    Index: ";
                if (loc.index.empty()) {
                    std::cout << RED << "Empty" << RESET;
                } else {
					for (size_t j = 0; j < loc.index.size(); ++j) {
                            std::cout << loc.index[j];
                            if (j < loc.index.size() - 1) std::cout << ", ";
                        }
                        std::cout << std::endl;
                }
                std::cout << std::endl;
                
                std::cout << "    Autoindex: ";
                if (loc.autoindex) {
                    std::cout << GREEN << "ON" << RESET;
                } else {
                    std::cout << RED << "OFF" << RESET;
                }
                std::cout << std::endl;
                
                std::cout << "    Upload: ";
                if (loc.allow_upload) {
                    std::cout << GREEN << "ON" << RESET;
                } else {
                    std::cout << RED << "OFF" << RESET;
                }
                std::cout << std::endl;
                
                if (loc.allow_upload && !loc.upload_dir.empty()) {
                    std::cout << "    Upload Dir: " << loc.upload_dir << std::endl;
                }
                
                if (!loc.return_path.empty()) {
                    std::cout << "    Redirect: " << YELLOW << loc.return_path << RESET << std::endl;
                }
                
                // Methods
                std::cout << "    Methods: ";
                if (loc.allow_methods.empty()) {
                    std::cout << RED << "Empty" << RESET << std::endl;
                } else {
                    for (size_t j = 0; j < loc.allow_methods.size(); ++j) {
                        std::cout << loc.allow_methods[j];
                        if (j < loc.allow_methods.size() - 1) std::cout << ", ";
                    }
                    std::cout << std::endl;
                }
                
                // CGI
                if (!loc.cgi.empty()) {
                    std::cout << "    CGI: ";
                    std::map<std::string, std::string>::const_iterator cgi_it = loc.cgi.begin();
                    while (cgi_it != loc.cgi.end()) {
                        std::cout << cgi_it->first << " -> " << cgi_it->second;
                        ++cgi_it;
                        if (cgi_it != loc.cgi.end()) std::cout << ", ";
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
    
    std::cout << GREEN << "\n=== END SERVERS CONFIG DEBUG ===" << RESET << std::endl;
}

// FUNCIÓN PARA DEBUG DE SERVERS MANAGER
void showServers(const ServersManager& ws) {
    std::cout << GREEN << "\n=== SERVERS MANAGER DEBUG ===" << RESET << std::endl;

    size_t server_count = 1;
    std::map<Listen, std::vector<Servers> >::const_iterator ws_it;
    for (ws_it = ws.serversManager.begin(); ws_it != ws.serversManager.end(); ++ws_it) {
        const std::vector<Servers>& servers_vec = ws_it->second;
        std::vector<Servers>::const_iterator s_it;
        for (s_it = servers_vec.begin(); s_it != servers_vec.end(); ++s_it, ++server_count) {
            const Servers& srv = *s_it;
            
            std::cout << BLUE << "\n--- Server " << server_count << " ---" << RESET << std::endl;
            
        // Listen
            std::cout << YELLOW << "Listen Directive:" << RESET << std::endl;
            const Listen& listen = srv.getListens();
            std::cout << "  - " << listen.to_string();
            if (listen.is_default) {
                std::cout << GREEN << " [DEFAULT_SERVER]" << RESET;
            }
            std::cout << std::endl;
            
            // Server names
            std::cout << YELLOW << "Server Names:" << RESET << std::endl;
            const std::vector<std::string>& server_names = srv.getServerNames();
            if (server_names.empty()) {
                std::cout << "  - " << RED << "Empty" << RESET << std::endl;
            } else {
                for (size_t j = 0; j < server_names.size(); ++j) {
                    std::cout << "  - " << server_names[j] << std::endl;
                }
            }
            
            // Root
            std::cout << YELLOW << "Root: " << RESET << srv.getRoot() << std::endl;
            
            // Index files
            std::cout << YELLOW << "Index Files:" << RESET << std::endl;
            const std::vector<std::string>& index_files = srv.getIndexFiles();
            if (index_files.empty()) {
                std::cout << "  - " << RED << "Empty" << RESET << std::endl;
            } else {
                for (size_t j = 0; j < index_files.size(); ++j) {
                    std::cout << "  - " << index_files[j] << std::endl;
                }
            }
            
            // Error pages
            std::cout << YELLOW << "Error Pages:" << RESET << std::endl;
            const std::map<int, std::string>& error_pages = srv.getErrorPages();
            if (error_pages.empty()) {
                std::cout << "  - " << RED << "Empty" << RESET << std::endl;
            } else {
                for (std::map<int, std::string>::const_iterator it = error_pages.begin();
                     it != error_pages.end(); ++it) {
                    std::cout << "  - " << it->first << " -> " << it->second << std::endl;
                }
            }
            
            // Allow methods
            std::cout << YELLOW << "Allow Methods:" << RESET << std::endl;
            const std::vector<std::string>& allow_methods = srv.getAllowMethods();
            if (allow_methods.empty()) {
                std::cout << "  - " << RED << "Empty" << RESET << std::endl;
            } else {
                for (size_t j = 0; j < allow_methods.size(); ++j) {
                    std::cout << "  - " << allow_methods[j] << std::endl;
                }
            }
            
            // Autoindex
            std::cout << YELLOW << "Autoindex: " << RESET;
            if (srv.getAutoindex()) {
                std::cout << GREEN << "ON" << RESET;
            } else {
                std::cout << RED << "OFF" << RESET;
            }
            std::cout << std::endl;
            
            // Client max body size
            std::cout << YELLOW << "Client Max Body Size: " << RESET << srv.getClientMaxBodySize() << std::endl;
            
            // Locations
            std::cout << YELLOW << "Locations:" << RESET << std::endl;
            const std::map<std::string, Locations>& locations = srv.getLocations();
            if (locations.empty()) {
                std::cout << "  - " << RED << "Empty" << RESET << std::endl;
            } else {
                for (std::map<std::string, Locations>::const_iterator it = locations.begin();
                     it != locations.end(); ++it) {
                    const std::string& path = it->first;
                    const Locations& loc = it->second;
                    
                    std::cout << "  - Path: " << YELLOW << path << RESET << std::endl;
                    
                    std::cout << "    Root: ";
                    if (loc.root.empty()) {
                        std::cout << RED << "Empty" << RESET;
                    } else {
                        std::cout << loc.root;
                    }
                    std::cout << std::endl;
                    
                    std::cout << "    Index: ";
                    if (loc.index.empty()) {
                        std::cout << RED << "Empty" << RESET;
                    } else {
						for (size_t j = 0; j < loc.index.size(); ++j) {
                            std::cout << loc.index[j];
                            if (j < loc.index.size() - 1) std::cout << ", ";
						}
                        std::cout << std::endl;
                    }
                    std::cout << std::endl;
                    
                    std::cout << "    Autoindex: ";
                    if (loc.autoindex) {
                        std::cout << GREEN << "ON" << RESET;
                    } else {
                        std::cout << RED << "OFF" << RESET;
                    }
                    std::cout << std::endl;
                    
                    std::cout << "    Upload: ";
                    if (loc.allow_upload) {
                        std::cout << GREEN << "ON" << RESET;
                    } else {
                        std::cout << RED << "OFF" << RESET;
                    }
                    std::cout << std::endl;
                    
                    if (loc.allow_upload && !loc.upload_dir.empty()) {
                        std::cout << "    Upload Dir: " << loc.upload_dir << std::endl;
                    }
                    
                    if (!loc.return_path.empty()) {
                        std::cout << "    Redirect: " << YELLOW << loc.return_path << RESET << std::endl;
                    }
                    
                    // Methods
                    std::cout << "    Methods: ";
                    if (loc.allow_methods.empty()) {
                        std::cout << RED << "Empty" << RESET << std::endl;
                    } else {
                        for (size_t j = 0; j < loc.allow_methods.size(); ++j) {
                            std::cout << loc.allow_methods[j];
                            if (j < loc.allow_methods.size() - 1) std::cout << ", ";
                        }
                        std::cout << std::endl;
                    }
                    
                    // CGI
                    if (!loc.cgi.empty()) {
                        std::cout << "    CGI: ";
                        std::map<std::string, std::string>::const_iterator cgi_it = loc.cgi.begin();
                        while (cgi_it != loc.cgi.end()) {
                            std::cout << cgi_it->first << " -> " << cgi_it->second;
                            ++cgi_it;
                            if (cgi_it != loc.cgi.end()) std::cout << ", ";
                        }
                        std::cout << std::endl;
                    }
                }
            }
        }
    }

    std::cout << GREEN << "\n=== END SERVERS MANAGER DEBUG ===" << RESET << std::endl;
}
