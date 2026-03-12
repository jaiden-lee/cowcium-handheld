#include "gpio_power.h"

GpioPower::GpioPower() : request_(nullptr) {}

GpioPower::~GpioPower() {
    if (request_ != nullptr) {
        gpiod_line_request_release(request_);
    }
}

bool GpioPower::enable(unsigned int bcm_line_num) {
    gpiod_chip* chip = gpiod_chip_open("/dev/gpiochip0");
    if (chip == nullptr) {
        return false;
    }

    gpiod_line_settings* settings = gpiod_line_settings_new();
    if (settings == nullptr) {
        gpiod_chip_close(chip);
        return false;
    }

    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_ACTIVE);

    gpiod_line_config* line_config = gpiod_line_config_new();
    if (line_config == nullptr) {
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return false;
    }

    unsigned int offsets[1] = {bcm_line_num};
    if (gpiod_line_config_add_line_settings(line_config, offsets, 1, settings) < 0) {
        gpiod_line_config_free(line_config);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return false;
    }

    gpiod_request_config* request_config = gpiod_request_config_new();
    if (request_config == nullptr) {
        gpiod_line_config_free(line_config);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return false;
    }

    gpiod_request_config_set_consumer(request_config, "cowcium-local-server");
    request_ = gpiod_chip_request_lines(chip, request_config, line_config);

    gpiod_request_config_free(request_config);
    gpiod_line_config_free(line_config);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);

    return request_ != nullptr;
}
