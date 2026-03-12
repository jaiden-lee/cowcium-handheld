#pragma once

#include "color_sensor.h"

class HttpServer {
public:
    HttpServer(int port, ColorSensor& sensor);
    bool run() const;

private:
    int port_;
    ColorSensor& sensor_;
};
