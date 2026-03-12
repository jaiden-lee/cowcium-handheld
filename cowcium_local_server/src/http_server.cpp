#include "http_server.h"

#include <httplib.h>

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

    server.Get("/poll", [this](const httplib::Request&, httplib::Response& response) {
        const ColorReading reading = sensor_.read_color();
        std::cout
            << "event=request method=GET path=/poll"
            << " clear=" << reading.clear
            << " red=" << reading.red
            << " green=" << reading.green
            << " blue=" << reading.blue
            << " red_normalized=" << reading.red_normalized
            << " green_normalized=" << reading.green_normalized
            << " blue_normalized=" << reading.blue_normalized
            << std::endl;
        response.set_content(build_json(reading), "application/json");
    });

    std::cout << "event=server_listening host=0.0.0.0 port=" << port_ << std::endl;
    return server.listen("0.0.0.0", port_);
}
