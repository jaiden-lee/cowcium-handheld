#include "http_server.h"

#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

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
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        return false;
    }

    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(static_cast<uint16_t>(port_));

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        close(server_fd);
        return false;
    }

    if (listen(server_fd, 8) < 0) {
        close(server_fd);
        return false;
    }

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            continue;
        }

        char buffer[2048];
        const ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';

            const bool is_poll = std::strncmp(buffer, "GET /poll ", 10) == 0;
            if (is_poll) {
                const ColorReading reading = sensor_.read_color();
                const std::string body = build_json(reading);

                std::ostringstream response;
                response << "HTTP/1.1 200 OK\r\n"
                         << "Content-Type: application/json\r\n"
                         << "Content-Length: " << body.size() << "\r\n"
                         << "Connection: close\r\n"
                         << "\r\n"
                         << body;

                const std::string response_text = response.str();
                write(client_fd, response_text.c_str(), response_text.size());
            } else {
                static constexpr char not_found[] =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n";
                write(client_fd, not_found, sizeof(not_found) - 1);
            }
        }

        close(client_fd);
    }

    close(server_fd);
    return true;
}
