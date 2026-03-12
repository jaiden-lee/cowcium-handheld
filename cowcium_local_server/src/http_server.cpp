#include "http_server.h"

#include <httplib.h>

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
        response.set_content(build_json(reading), "application/json");
    });

    return server.listen("0.0.0.0", port_);
}
