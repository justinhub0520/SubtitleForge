#include "server.h"
#include "config.h"
#include <iostream>
#include <csignal>

int main() {
    auto& config = subforge::Config::instance();

    std::cout << "SubForge Backend v1.0.0" << std::endl;
    std::cout << "API Key configured: " << (config.whisper_api_key().empty() ? "NO" : "YES") << std::endl;

    subforge::Server server;

    std::signal(SIGINT, [](int) {
        std::cout << "\nShutting down..." << std::endl;
    });

    server.start(config.server_host(), config.server_port());

    return 0;
}