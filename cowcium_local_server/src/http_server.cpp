#include "http_server.h"

#include <httplib.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {
std::string build_json(const ColorReading& reading) {
    std::ostringstream json;
    json << "{"
         << "\"clear\":" << reading.clear << ","
         << "\"red\":" << reading.red << ","
         << "\"green\":" << reading.green << ","
         << "\"blue\":" << reading.blue << ","
         << "\"redNormalized\":" << reading.red_normalized << ","
         << "\"greenNormalized\":" << reading.green_normalized << ","
         << "\"blueNormalized\":" << reading.blue_normalized
         << "}";
    return json.str();
}
}

HttpServer::HttpServer(int port, ColorSensor& sensor) : port_(port), sensor_(sensor) {}

bool HttpServer::run() const {
    httplib::Server server;

    server.set_default_headers({
        {"Access-Control-Allow-Origin",  "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"}
    });

    server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::ifstream f("index.html");
        if (!f.is_open()) {
            res.status = 404;
            res.set_content("index.html not found", "text/plain");
        } else {
            std::string html((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
            res.set_content(html, "text/html");
        }
    });

    server.Post("/record", [](const httplib::Request& req, httplib::Response& res) {
        std::cerr << "event=request method=POST path=/record body=" << req.body << std::endl;
        res.set_content("{\"ok\":true}", "application/json");
    });
    server.Get("/poll", [this](const httplib::Request&, httplib::Response& response) {
    const ColorReading reading = sensor_.read_color();
    std::cerr
        << "event=request method=GET path=/poll"
        << " clear=" << reading.clear
        << " red=" << reading.red
        << " green=" << reading.green
        << " blue=" << reading.blue
        << " red_normalized=" << reading.red_normalized
        << " green_normalized=" << reading.green_normalized
        << " blue_normalized=" << reading.blue_normalized
        << std::endl;

    response.set_header("Access-Control-Allow-Origin", "*");
    response.set_content(build_json(reading), "application/json");
});

    std::cerr << "event=startup stage=http_server_bind_begin host=0.0.0.0 port=" << port_ << std::endl;
    if (!server.bind_to_port("0.0.0.0", port_)) {
        std::cerr << "event=error stage=http_server_bind host=0.0.0.0 port=" << port_ << std::endl;
        return false;
    }

    std::cerr << "event=server_listening host=0.0.0.0 port=" << port_ << std::endl;
    return server.listen_after_bind();
}
