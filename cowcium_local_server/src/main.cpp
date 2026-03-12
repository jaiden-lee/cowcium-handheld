#include "color_sensor.h"
#include "gpio_power.h"
#include "http_server.h"

#include <iostream>
#include <unistd.h>

int main() {
    std::cerr << "event=startup stage=gpio_power_enable_begin" << std::endl;
    GpioPower power;
    if (!power.enable(5)) {
        std::cerr << "event=error stage=gpio_power_enable" << std::endl;
        return 1;
    }

    std::cerr << "event=startup stage=gpio_power_enable_complete" << std::endl;
    usleep(500000);

    std::cerr << "event=startup stage=color_sensor_init_begin device=/dev/i2c-0 address=0x29" << std::endl;
    ColorSensor sensor;
    if (!sensor.initialize("/dev/i2c-0", 0x29)) {
        std::cerr << "event=error stage=color_sensor_init" << std::endl;
        return 1;
    }

    std::cerr << "event=startup stage=color_sensor_init_complete" << std::endl;
    HttpServer server(8080, sensor);
    std::cerr << "event=startup stage=http_server_run_begin port=8080" << std::endl;
    if (!server.run()) {
        std::cerr << "event=error stage=http_server_run port=8080" << std::endl;
        return 1;
    }

    return 0;
}
