#ifndef I2C_H
#define I2C_H

// Read and write direction constants.
const uint8_t WRITE = 0;
const uint8_t READ = 1;

/**
 * Configures and enables the SERCOM I2C peripheral to communicate in Master mode, at a standard baudrate of 100kHz, with no timeouts, and smart mode enabled.
 **/
void i2c_init();

/**
 * Performs a blocking read or write transaction to a secondary device across I2C.
 * 
 * PARAMS:
 * uint8_t  address -> Address of secondary device to read from/write to.
 * uint8_t  dir     -> Direction of transaction (0 for write, 1 for read, else nothing).
 * uint8_t* data    -> Array of data to read from/write to across I2C.
 * uint8_t  len     -> Number of bytes of `data` to read from/write to across I2C.
 * 
 * RETURNS:
 * uint8_t          -> Number of bytes read from/written to across I2C.
 **/
uint8_t i2c_transaction(uint8_t address, uint8_t dir, uint8_t* data, uint8_t len);

/**
 * Issues an I2C STOP bit. 
 * 
 * NOTE: I2C read transaction will automatically send a STOP bit, `i2c_send_stop()` should be used ONLY after an I2C write transaction.
 **/
void i2c_send_stop();

#endif
