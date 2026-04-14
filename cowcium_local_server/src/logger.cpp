#include "logger.h"

#include <fstream>
#include <iostream>
#include <mutex>

namespace {
std::mutex& log_mutex() {
    static std::mutex mutex;
    return mutex;
}
}

void log_line(const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex());

    std::cerr << message << std::endl;

    std::ofstream file("cowcium_local_server.log", std::ios::app);
    if (file.is_open()) {
        file << message << '\n';
    }
}
