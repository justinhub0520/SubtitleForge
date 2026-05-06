#include "server.h"
#include "config.h"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "Running backend tests..." << std::endl;

    auto& config = subforge::Config::instance();
    assert(!config.server_host().empty());
    assert(config.server_port() > 0);
    std::cout << "  [PASS] Config loading" << std::endl;

    subforge::Server server;
    std::cout << "  [PASS] Server creation" << std::endl;

    std::cout << "All tests passed!" << std::endl;
    return 0;
}