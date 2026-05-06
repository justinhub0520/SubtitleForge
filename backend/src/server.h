#pragma once

#include "httplib.h"
#include <string>

namespace subforge {

class Server {
public:
    Server();
    void start(const std::string& host, int port);
    void stop();

private:
    void setup_routes();
    void handle_health(const httplib::Request& req, httplib::Response& res);

    httplib::Server server_;
    bool running_ = false;
};

} // namespace subforge