#include "color_sensor.h"

#include <fcntl.h>
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
        return false;
    }

    if (ioctl(fd_, I2C_SLAVE, address) < 0) {
        return false;
    }

    write_config_registers();
    return true;
}

ColorReading ColorSensor::read_color() const {
    ColorReading reading{};
    reading.clear = read_register16(0x94);
    reading.red = read_register16(0x96);
    reading.green = read_register16(0x98);
    reading.blue = read_register16(0x9A);

    const float clear_value = reading.clear == 0 ? 1.0f : static_cast<float>(reading.clear);
    reading.red_normalized = static_cast<float>(reading.red) / clear_value;
    reading.green_normalized = static_cast<float>(reading.green) / clear_value;
    reading.blue_normalized = static_cast<float>(reading.blue) / clear_value;

    return reading;
}

void ColorSensor::write_config_registers() const {
    constexpr uint8_t register_offset = 0x80;
    constexpr uint8_t enable_register = 0x00 | register_offset;
    constexpr uint8_t integration_time_register = 0x01 | register_offset;
    constexpr uint8_t sensitivity_register = 0x0F | register_offset;

    uint8_t write_params[2];

    write_params[0] = integration_time_register;
    write_params[1] = 0xD5;
    write(fd_, write_params, 2);

    write_params[0] = sensitivity_register;
    write_params[1] = 0x02;
    write(fd_, write_params, 2);

    write_params[0] = enable_register;
    write_params[1] = 0x01;
    write(fd_, write_params, 2);
    usleep(10000);

    write_params[0] = enable_register;
    write_params[1] = 0x03;
    write(fd_, write_params, 2);
    usleep(200000);
}

uint16_t ColorSensor::read_register16(uint8_t reg) const {
    uint8_t register_address = reg;
    write(fd_, &register_address, 1);

    uint8_t data[2];
    read(fd_, data, 2);

    return static_cast<uint16_t>((data[1] << 8) | data[0]);
}
