#ifndef BMA253_H
#define BMA253_H

// BMA253 address and configuration constants
const uint8_t BMA253_ADDR = 0x19;
const uint8_t BW_ADDR = 0x10;
const uint8_t BW_VAL = 0x8;       // Slowest bandwidth, 7.81Hz or 64ms update time.
const uint8_t RES_ADDR = 0xF;
const uint8_t RES_VAL = 0xC;      // Highest range, +/-16g with 7.81mg/LSB resolution.
const uint8_t X_LSB = 0x3;
const uint8_t X_MSB = 0x2;
const uint8_t Y_LSB = 0x5;
const uint8_t Y_MSB = 0x4;
const uint8_t Z_LSB = 0x7;
const uint8_t Z_MSB = 0x6;
const int X = 0;
const int Y = 1;
const int Z = 2;
const int LSB = 0;
const int MSB = 1;
const int DATA_LEN = 2;

/**
 * Configure and enable the BMA250 accelerometer.
 **/
void bma253_init();

/**
 * Read the current X, Y, and Z acceleration data from the BMA253 accelerometer. 
 * The acceleration data will reflect the orientation and movement of the hardware.
 *
 * PARAMS:
 * int16_t* x   -> x-axis acceleration value.
 * int16_t* y   -> y-axis acceleration value.
 * int16_t* z   -> z-axis acceleration value.
 **/
void bma253_read_xyz(int16_t* x, int16_t* y, int16_t* z);

/**
 * Convert BMA253 acceleration register data to a signed 16-bit integer.
 *
 * PARAMS:
 * uint8_t LSB  -> BMA253 LSB acceleration data.
 * uint8_t MSB  -> BMA253 MSB acceleration data.
 *
 * RETURNS:
 * int16_t      -> Signed 16-bit acceleration count @ 7.81mg/LSB.
 **/
int16_t bma253_convert_xyz(uint8_t LSB, uint8_t MSB);

#endif