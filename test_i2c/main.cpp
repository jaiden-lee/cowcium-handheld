#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstdint>



void writeToConfigRegisters(int file);
uint16_t readRegister16(int file, uint8_t reg);

int main() {
    int file = open("/dev/i2c-1", O_RDWR);
    if (file < 0) {
        std::cerr << "Failed to open I2C bus\n";
        return 1;
    }

    // 2. Tell Linux which device we want (0x29)
    int addr = 0x29;
    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        std::cerr << "Failed to select I2C device\n";
        return 1;
    }

    writeToConfigRegisters(file);
    
    while (true) {
        uint16_t clear = readRegister16(file, 0x94);
        uint16_t red   = readRegister16(file, 0x96);
        uint16_t green = readRegister16(file, 0x98);
        uint16_t blue  = readRegister16(file, 0x9A);

        if (clear == 0) clear = 1;

        float rNorm = (float)red / clear;
        float gNorm = (float)green / clear;
        float bNorm = (float)blue / clear;

        std::cout << "R=" << rNorm
              << " G=" << gNorm
              << " B=" << bNorm
              << std::endl;
            
        usleep(500000);  // read every 500ms
    }


    close(file);
    return 0;
}

void writeToConfigRegisters(int file) {
    uint8_t regOffset = 0x80;

    // Write to CONFIG registers
    uint8_t enableReg = 0x00 | regOffset;
    uint8_t integrationTimeReg = 0x01 | regOffset;
    uint8_t sensitivityReg = 0x0F | regOffset;

    // write integration time
    uint8_t writeParams[2];
    writeParams[0] = integrationTimeReg;
    writeParams[1] = 0xD5;
    write(file, writeParams, 2); // pointer to write params buffer

    // write sensitivity
    writeParams[0] = sensitivityReg;
    writeParams[1] = 0x02;
    write(file, writeParams, 2);

    // write enable
    writeParams[0] = enableReg;
    writeParams[1] = 0x01;
    write(file, writeParams, 2);

    usleep(10000); // 10ms

    // enable ADC
    writeParams[0] = enableReg;
    writeParams[1] = 0x03;
    write(file, writeParams, 2);
    usleep(200000); // 200ms
}

uint16_t readRegister16(int file, uint8_t reg) {
    uint8_t regAddr = reg;
    write(file, &regAddr, 1);


    // register numbers are lowbyte, then high byte
    uint8_t data[2];
    read(file, data, 2);

    uint16_t value = (data[1] << 8) | data[0];

    return value;
}