#include "color_sensor.h"

#include <fcntl.h>
#include <iostream>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

ColorSensor::ColorSensor() : fd_(-1) {}

ColorSensor::~ColorSensor() {
    if (fd_ >= 0) {
        close(fd_);
    }
}

bool ColorSensor::initialize(const std::string& i2c_device, int address) {
    fd_ = open(i2c_device.c_str(), O_RDWR);
    if (fd_ < 0) {
        std::cerr << "event=error stage=color_sensor_open device=" << i2c_device << std::endl;
        return false;
    }

    if (ioctl(fd_, I2C_SLAVE, address) < 0) {
        std::cerr << "event=error stage=color_sensor_select address=0x29" << std::endl;
        return false;
    }

    for (int attempt = 1; attempt <= 5; ++attempt) {
        std::cerr << "event=startup stage=color_sensor_config_attempt attempt=" << attempt << std::endl;
        if (write_config_registers()) {
            for (int warmup_attempt = 1; warmup_attempt <= 5; ++warmup_attempt) {
                ColorReading reading{};
                reading.clear = read_register16(0x94);
                reading.red = read_register16(0x96);
                reading.green = read_register16(0x98);
                reading.blue = read_register16(0x9A);

                if (has_signal(reading)) {
                    const float clear_value = reading.clear == 0 ? 1.0f : static_cast<float>(reading.clear);
                    reading.red_normalized = static_cast<float>(reading.red) / clear_value;
                    reading.green_normalized = static_cast<float>(reading.green) / clear_value;
                    reading.blue_normalized = static_cast<float>(reading.blue) / clear_value;
                    last_good_reading_ = reading;
                    std::cerr << "event=startup stage=color_sensor_warmup_complete attempt=" << warmup_attempt << std::endl;
                    return true;
                }

                usleep(200000);
            }
        }

        usleep(200000);
    }

    std::cerr << "event=error stage=color_sensor_config" << std::endl;
    return false;
}

ColorReading ColorSensor::read_color() const {
    std::lock_guard<std::mutex> lock(mutex_);

    ColorReading reading{};

    for (int attempt = 0; attempt < 3; ++attempt) {
        reading.clear = read_register16(0x94);
        reading.red = read_register16(0x96);
        reading.green = read_register16(0x98);
        reading.blue = read_register16(0x9A);

        if (has_signal(reading)) {
            break;
        }

        write_config_registers();
        usleep(200000);
    }

    const float clear_value = reading.clear == 0 ? 1.0f : static_cast<float>(reading.clear);
    reading.red_normalized = static_cast<float>(reading.red) / clear_value;
    reading.green_normalized = static_cast<float>(reading.green) / clear_value;
    reading.blue_normalized = static_cast<float>(reading.blue) / clear_value;

    if (has_signal(reading)) {
        last_good_reading_ = reading;
        return reading;
    }

    if (last_good_reading_.has_value()) {
        return *last_good_reading_;
    }

    return reading;
}

bool ColorSensor::write_config_registers() const {
    constexpr uint8_t register_offset = 0x80;
    constexpr uint8_t enable_register = 0x00 | register_offset;
    constexpr uint8_t integration_time_register = 0x01 | register_offset;
    constexpr uint8_t sensitivity_register = 0x0F | register_offset;

    if (!write_register(integration_time_register, 0xD5)) {
        return false;
    }

    if (!write_register(sensitivity_register, 0x02)) {
        return false;
    }

    if (!write_register(enable_register, 0x01)) {
        return false;
    }
    usleep(10000);

    if (!write_register(enable_register, 0x03)) {
        return false;
    }

    usleep(200000);
    return true;
}

bool ColorSensor::write_register(uint8_t reg, uint8_t value) const {
    uint8_t write_params[2] = {reg, value};
    const bool ok = write(fd_, write_params, 2) == 2;
    if (!ok) {
        std::cerr << "event=error stage=color_sensor_write_register reg=0x"
                  << std::hex << static_cast<int>(reg)
                  << " value=0x" << static_cast<int>(value)
                  << std::dec << std::endl;
    }
    return ok;
}

bool ColorSensor::has_signal(const ColorReading& reading) const {
    return reading.clear != 0 || reading.red != 0 || reading.green != 0 || reading.blue != 0;
}

uint16_t ColorSensor::read_register16(uint8_t reg) const {
    uint8_t register_address = reg;
    if (write(fd_, &register_address, 1) != 1) {
        return 0;
    }

    uint8_t data[2] = {0, 0};
    if (read(fd_, data, 2) != 2) {
        return 0;
    }

    return static_cast<uint16_t>((data[1] << 8) | data[0]);
}
