#pragma once

#include <cstdint>
#include <mutex>
#include <string>

struct ColorReading {
    uint16_t clear;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    float red_normalized;
    float green_normalized;
    float blue_normalized;
};

class ColorSensor {
public:
    ColorSensor();
    ~ColorSensor();

    bool initialize(const std::string& i2c_device, int address);
    ColorReading read_color() const;

private:
    bool write_config_registers() const;
    uint16_t read_register16(uint8_t reg) const;
    bool write_register(uint8_t reg, uint8_t value) const;

    int fd_;
    mutable std::mutex mutex_;
};
