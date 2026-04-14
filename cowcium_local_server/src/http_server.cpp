#include "http_server.h"
#include "logger.h"

#include <httplib.h>

#include <chrono>
#include <fstream>
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

    server.Get("/", [](const httplib::Request&, httplib::Response& response) {
        std::ifstream file("../src/index.html");
        if (!file.is_open()) {
            file.open("src/index.html");
        }
        if (!file.is_open()) {
            file.open("index.html");
        }

        if (!file.is_open()) {
            response.status = 404;
            response.set_content("index.html not found", "text/plain");
            return;
        }

        std::string html((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        response.set_content(html, "text/html");
    });

    server.Post("/record", [](const httplib::Request& request, httplib::Response& response) {
        log_line("event=request method=POST path=/record body=" + request.body);
        response.set_content("{\"ok\":true}", "application/json");
    });

    server.Get("/poll", [this](const httplib::Request&, httplib::Response& response) {
        const ColorReading reading = sensor_.read_color();
        std::ostringstream message;
        message << "event=request method=GET path=/poll"
                << " clear=" << reading.clear
                << " red=" << reading.red
                << " green=" << reading.green
                << " blue=" << reading.blue
                << " red_normalized=" << reading.red_normalized
                << " green_normalized=" << reading.green_normalized
                << " blue_normalized=" << reading.blue_normalized;
        log_line(message.str());
        response.set_content(build_json(reading), "application/json");
    });

    server.Get("/record-reading", [this](const httplib::Request&, httplib::Response& response) {
        const auto started_at = std::chrono::steady_clock::now();
        const ColorReading reading = sensor_.capture_stable_color(3000, 100);
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - started_at).count();
        std::ostringstream message;
        message << "event=request method=GET path=/record-reading"
                << " clear=" << reading.clear
                << " red=" << reading.red
                << " green=" << reading.green
                << " blue=" << reading.blue
                << " red_normalized=" << reading.red_normalized
                << " green_normalized=" << reading.green_normalized
                << " blue_normalized=" << reading.blue_normalized
                << " mode=median_filtered"
                << " elapsed_ms=" << elapsed_ms;
        log_line(message.str());
        response.set_content(build_json(reading), "application/json");
    });

    log_line("event=startup stage=http_server_bind_begin host=0.0.0.0 port=" + std::to_string(port_));
    if (!server.bind_to_port("0.0.0.0", port_)) {
        log_line("event=error stage=http_server_bind host=0.0.0.0 port=" + std::to_string(port_));
        return false;
    }

    log_line("event=server_listening host=0.0.0.0 port=" + std::to_string(port_));
    return server.listen_after_bind();
}
