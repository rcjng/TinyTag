#include "timer.h"

void timer3_init() {
  // Set TC3 Interrupt Priority to Level 0.
  NVIC_SetPriority(TC3_IRQn, 0);

  // Enable TC3 NVIC Interrupt Line.
  NVIC_EnableIRQ(TC3_IRQn);

  // Enable Interrupts.
  void __enable_irq(void);

  // Configure TC3 clock source.
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TCC2_TC3_Val;     // Select TC3 peripheral channel.
  GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_GEN_GCLK1;          // Use GCLK1.
  GCLK->CLKCTRL.bit.CLKEN = 1;                          // Enable TC3 generic clock.

  // Configure synchronous bus clock.
  PM->APBCSEL.bit.APBCDIV = 0;                          // Prescaler = 1.
  PM->APBCMASK.bit.TC3_ = 1;                            // Enable TC3 interface.

  // Configure Count Mode (16-bit).
  TC3->COUNT16.CTRLA.bit.MODE = 0x0;

  TC3->COUNT16.CTRLA.bit.PRESCALER = 0x5;               // Timer Prescaler = 64.

  // Configure TC3 Compare Mode for Compare Channel 0.
  TC3->COUNT16.CTRLA.bit.WAVEGEN = 0x1;                 // "Match Frequency" operation.

  // Enable TC3 compare mode interrupt generation.
  TC3->COUNT16.INTENSET.bit.MC0 = 0x1;                  // Enable match interrupts on compare channel 0.

  // Enable TC3.
  TC3->COUNT16.CTRLA.bit.ENABLE = 1;
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY == 1);
}

void timer3_reset() {
  // Reset timer counter to 0.
  TC3->COUNT16.COUNT.reg = 0x0;
}

void timer3_set(uint16_t period_ms) {
  // Timer must be disabled to set period, disable timer.
  TC3->COUNT16.CTRLA.bit.ENABLE = 0;
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY == 1);

  // Set Compare Counter to count equivalent of `period_ms`.
  TC3->COUNT16.CC[0].reg = (period_ms * TC3_PRESCALED_CLOCK_FREQUENCY) / NUM_MS_IN_SEC;

  // Re-enable timer.
  TC3->COUNT16.CTRLA.bit.ENABLE = 1;
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY == 1);
}