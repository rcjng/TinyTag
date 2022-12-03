#include "i2c.h"
#include "ledcircle.h"

void i2c_init() {
  // Configure PORT Pins PA22 and PA23 to use SERCOM3-PAD0 and SERCOM3-PAD1 respectively and peripheral function C.
  PORT->Group[0].PINCFG[22].bit.PMUXEN = 0x1;
  PORT->Group[0].PINCFG[23].bit.PMUXEN = 0x1;
  PORT->Group[0].PMUX[11].bit.PMUXE = 0x2;
  PORT->Group[0].PMUX[11].bit.PMUXO = 0x2;

  // Use SERCOM3 clock source.
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_SERCOM3_CORE_Val; // Select SERCOM3 peripheral channel.
  GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_GEN_GCLK0;          // Use GCLK0.
  GCLK->CLKCTRL.bit.CLKEN = 1;                          // Enable SERCOM3 generic clock.

  // Prescaler = 1.
  PM->APBCSEL.bit.APBCDIV = 0x0;

  // Set Master Mode.
  SERCOM3->I2CM.CTRLA.bit.MODE = 0x5;

  // Turn off timeouts.
  SERCOM3->I2CM.CTRLA.bit.INACTOUT = 0x0;

  // Enable smart-mode.
  SERCOM3->I2CM.CTRLB.bit.SMEN = 0x1;

  // Set baud for ~100kHz SCL (in actuality it's around ~100.5kHz): BAUD = ((f_GLCK/f_SCL) - (f_GLCK * T_RISE) - 10) / 2 = (2M/100K - 2M*230ns - 10) / 2 = 4.77.
  SERCOM3->I2CM.BAUD.bit.BAUD = 5;

  // Enable I2C.
  SERCOM3->I2CM.CTRLA.bit.ENABLE = 0x1;
  while(SERCOM3->I2CM.SYNCBUSY.bit.ENABLE);

  // Transition from "Unknown" state to "Idle" state.
  SERCOM3->I2CM.STATUS.bit.BUSSTATE = 0x1;
  while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
}

uint8_t i2c_transaction(uint8_t address, uint8_t dir, uint8_t* data, uint8_t len) {
  // Handle WRITE transaction.
  if (dir == 0) {
    // Send address (repeatedly, if arbitration lost).
    do {
      SERCOM3->I2CM.ADDR.bit.ADDR = (address << 1) & 0xFE;
      while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);      
    } while(SERCOM3->I2CM.INTFLAG.bit.MB && SERCOM3->I2CM.STATUS.bit.ARBLOST);
      
    // Send STOP bit and return if NACK received from secondary.
    if (SERCOM3->I2CM.INTFLAG.bit.MB && SERCOM3->I2CM.STATUS.bit.RXNACK) {
      i2c_send_stop();
      return 0;
    }

    // Send `len` bytes from memory starting at address `data` across I2C.
    for (int i = 0; i < len ; i++) {
      // Send byte (repeatedly, if arbitration lost).
      do { 
        while(SERCOM3->I2CM.INTFLAG.bit.MB != 0x1 && SERCOM3->I2CM.STATUS.bit.CLKHOLD != 0x1);
        SERCOM3->I2CM.DATA.bit.DATA = *(data + i);
        while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
      } while (SERCOM3->I2CM.INTFLAG.bit.MB && SERCOM3->I2CM.STATUS.bit.ARBLOST);

      // Send STOP bit and return if NACK received from secondary.
      if (SERCOM3->I2CM.INTFLAG.bit.MB && SERCOM3->I2CM.STATUS.bit.RXNACK) {
        i2c_send_stop();
        return i+1;
      }
    }

    return len;
  // Handle READ transaction.
  } else if (dir == 1) {
    // Send address (repeatedly, if arbitration lost).
    do {
      SERCOM3->I2CM.ADDR.bit.ADDR = ((address << 1) & 0xFE) | 0x1;
      while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
    } while(SERCOM3->I2CM.INTFLAG.bit.MB && SERCOM3->I2CM.STATUS.bit.ARBLOST);

    // Send STOP bit and return if NACK received from secondary.
    if (SERCOM3->I2CM.INTFLAG.bit.MB && SERCOM3->I2CM.STATUS.bit.RXNACK) {
      i2c_send_stop();
      return 0;
    }

    // Read `len` bytes across I2C and store in memory starting at address `data`.
    for (int i = 0; i < len; i++) {
      // When reading the last byte.
      if (i == len-1) {
        // Set acknowledge action to NACK.
        SERCOM3->I2CM.CTRLB.bit.ACKACT = 1;
        while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);

        // Receive data (repeatedly, if arbitration lost).
        do {
          *(data + i) = SERCOM3->I2CM.DATA.reg;
          while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
          while(SERCOM3->I2CM.INTFLAG.bit.SB != 0x1 && SERCOM3->I2CM.STATUS.bit.CLKHOLD != 0x1);   
        } while(SERCOM3->I2CM.INTFLAG.bit.MB);

        // Issue NACK followed by a STOP bit.
        SERCOM3->I2CM.CTRLB.bit.CMD = 0x3;
        while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
      // When reading all other bytes.
      } else {
        // Set acknowledge action to ACK.
        SERCOM3->I2CM.CTRLB.bit.ACKACT = 0;
        while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);

        // Receive data (repeatedly, if arbitration lost).
        do {
          data[i] = SERCOM3->I2CM.DATA.reg;
          while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
          while(SERCOM3->I2CM.INTFLAG.bit.SB != 0x1 && SERCOM3->I2CM.STATUS.bit.CLKHOLD != 0x1);
        } while(SERCOM3->I2CM.INTFLAG.bit.MB);

        // Issue ACK followed by another byte read.
        SERCOM3->I2CM.CTRLB.bit.CMD = 0x2;
        while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
      }
    }
  // INVALID DIR, return 0 bytes read or written.
  } else {
    return 0;
  }
}

void i2c_send_stop() {
  // Issue acknowledge action followed by a STOP bit.
  while(SERCOM3->I2CM.INTFLAG.bit.MB != 0x1 && SERCOM3->I2CM.STATUS.bit.CLKHOLD != 0x1);
  SERCOM3->I2CM.CTRLB.bit.CMD = 0x3;
  while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
}
