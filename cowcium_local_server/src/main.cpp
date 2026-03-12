#include "color_sensor.h"
#include "gpio_power.h"
#include "http_server.h"

#include <unistd.h>

int main() {
    GpioPower power;
    if (!power.enable(5)) {
        return 1;
    }

    usleep(500000);

    ColorSensor sensor;
    if (!sensor.initialize("/dev/i2c-0", 0x29)) {
        return 1;
    }

    HttpServer server(8080, sensor);
    if (!server.run()) {
        return 1;
    }

    return 0;
}
