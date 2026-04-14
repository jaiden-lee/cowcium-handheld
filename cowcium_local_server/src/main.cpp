#include "color_sensor.h"
#include "gpio_power.h"
#include "http_server.h"
#include "logger.h"

#include <unistd.h>

int main() {
    log_line("event=startup stage=gpio_power_enable_begin");
    GpioPower power;
    if (!power.enable(5)) {
        log_line("event=error stage=gpio_power_enable");
        return 1;
    }

    log_line("event=startup stage=gpio_power_enable_complete");
    usleep(1000000);

    log_line("event=startup stage=color_sensor_init_begin device=/dev/i2c-0 address=0x29");
    ColorSensor sensor;
    if (!sensor.initialize("/dev/i2c-0", 0x29)) {
        log_line("event=error stage=color_sensor_init");
        return 1;
    }

    log_line("event=startup stage=color_sensor_init_complete");
    HttpServer server(8080, sensor);
    log_line("event=startup stage=http_server_run_begin port=8080");
    if (!server.run()) {
        log_line("event=error stage=http_server_run port=8080");
        return 1;
    }

    return 0;
}
