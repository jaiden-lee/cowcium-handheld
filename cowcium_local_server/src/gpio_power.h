#pragma once

#include <gpiod.h>

class GpioPower {
public:
    GpioPower();
    ~GpioPower();

    bool enable(unsigned int bcm_line_num);

private:
    gpiod_chip* chip_;
    gpiod_line* line_;
};
