#include "server.h"
#include "config.h"
#include <iostream>
#include <csignal>
#include <atomic>

static subforge::Server* g_server = nullptr;
static std::atomic<bool> g_running{true};

int main() {
    auto& config = subforge::Config::instance();

    std::cout << "SubForge Backend v1.0.0" << std::endl;
    std::cout << "API Key configured: " << (config.dashscope_api_key().empty() ? "NO" : "YES") << std::endl;

    subforge::Server server;
    g_server = &server;

    std::signal(SIGINT, [](int) {
        std::cout << "\nShutting down..." << std::endl;
        g_running = false;
        if (g_server) {
            g_server->stop();
        }
    });

    server.start(config.server_host(), config.server_port());

    return 0;
}