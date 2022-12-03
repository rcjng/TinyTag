#include "i2c.h"
#include "bma253.h"

/**
 * NOTE: My TinyZero board has a BMA253 so this file actually configures and reads acceleration data assuming a BMA253.
 **/

void bma253_init() {
  // Initialize data array for BMA253 configuration.
  uint8_t wdata_len = 2;
  uint8_t wdata[wdata_len];

  PM->APBCMASK.bit.SERCOM3_ = 1;                                                                                // Ungate SERCOM3 Clock

  // Write and configure BMA253 bandwidth to 64ms across I2C.
  wdata[0] = BW_ADDR;
  wdata[1] = BW_VAL;
  i2c_transaction(BMA253_ADDR, WRITE, wdata, wdata_len);
  i2c_send_stop();

  // Write and configure BMA253 range/resolution to +/-16g and 7.81mg/LSB across I2C.
  wdata[0] = RES_ADDR;
  wdata[1] = RES_VAL;
  i2c_transaction(BMA253_ADDR, WRITE, wdata, wdata_len);
  i2c_send_stop();

  PM->APBCMASK.bit.SERCOM3_ = 0;                                                                                // Gate SERCOM3 Clock
}

void bma253_read_xyz(int16_t* x, int16_t* y, int16_t* z) {
  // Set up read/write buffers and lengths.
  uint8_t wdata_len = 1;
  uint8_t wdata[wdata_len];
  uint8_t rdata_len = 6;
  uint8_t rdata[rdata_len];
  uint8_t read_len = 1;

  PM->APBCMASK.bit.SERCOM3_ = 1;                                                                                // Ungate SERCOM3 Clock

  // Read x-axis LSB acceleration data.
  wdata[0] = X_LSB;
  i2c_transaction(BMA253_ADDR, WRITE, wdata, wdata_len);
  i2c_transaction(BMA253_ADDR, READ, rdata + (DATA_LEN*X+LSB), read_len);

  // Read x-axis MSB acceleration data.
  wdata[0] = X_MSB;
  i2c_transaction(BMA253_ADDR, WRITE, wdata, wdata_len);
  i2c_transaction(BMA253_ADDR, READ, rdata + (DATA_LEN*X+MSB), read_len);

  // Read y-axis LSB acceleration data.
  wdata[0] = Y_LSB;
  i2c_transaction(BMA253_ADDR, WRITE, wdata, wdata_len);
  i2c_transaction(BMA253_ADDR, READ, rdata + (DATA_LEN*Y+LSB), read_len);

  // Read y-axis MSB acceleration data.
  wdata[0] = Y_MSB;
  i2c_transaction(BMA253_ADDR, WRITE, wdata, wdata_len);
  i2c_transaction(BMA253_ADDR, READ, rdata + (DATA_LEN*Y+MSB), read_len);

  // Read z-axis LSB acceleration data.
  wdata[0] = Z_LSB;
  i2c_transaction(BMA253_ADDR, WRITE, wdata, wdata_len);
  i2c_transaction(BMA253_ADDR, READ, rdata + (DATA_LEN*Z+LSB), read_len);

  // Read z-axis MSB acceleration data.
  wdata[0] = Z_MSB;
  i2c_transaction(BMA253_ADDR, WRITE, wdata, wdata_len);
  i2c_transaction(BMA253_ADDR, READ, rdata + (DATA_LEN*Z+MSB), read_len);

  PM->APBCMASK.bit.SERCOM3_ = 0;                                                                                // Gate SERCOM3 Clock

  *x = bma253_convert_xyz(rdata[DATA_LEN*X+LSB], rdata[DATA_LEN*X+MSB]);
  *y = bma253_convert_xyz(rdata[DATA_LEN*Y+LSB], rdata[DATA_LEN*Y+MSB]);
  *z = bma253_convert_xyz(rdata[DATA_LEN*Z+LSB], rdata[DATA_LEN*Z+MSB]);
}

int16_t bma253_convert_xyz(uint8_t l, uint8_t u) {
  // Include MSB data.
  int16_t accel = ((int16_t) u) << 8;
  accel = accel >> 4;

  // Include LSB data.
  accel = accel | (l >> 4);

  return accel;
}