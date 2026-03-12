#include "gpio_power.h"

GpioPower::GpioPower() : chip_(nullptr), line_(nullptr) {}

GpioPower::~GpioPower() {
    if (line_ != nullptr) {
        gpiod_line_release(line_);
    }

    if (chip_ != nullptr) {
        gpiod_chip_close(chip_);
    }
}

bool GpioPower::enable(unsigned int bcm_line_num) {
    chip_ = gpiod_chip_open("/dev/gpiochip0");
    if (chip_ == nullptr) {
        return false;
    }

    line_ = gpiod_chip_get_line(chip_, bcm_line_num);
    if (line_ == nullptr) {
        return false;
    }

    if (gpiod_line_request_output(line_, "cowcium-local-server", 1) < 0) {
        return false;
    }

    return gpiod_line_set_value(line_, 1) == 0;
}
