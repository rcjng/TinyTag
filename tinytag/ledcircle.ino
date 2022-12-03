#include "ledcircle.h"

void ledcircle_select(uint16_t led) {
  // Iterate over each LED in the LED array/bitmask.
  for (uint8_t i = 0; i < NUMBER_OF_LEDS; i++) {
    // Turn off LED
    PORT->Group[0].DIRCLR.reg = PORT_PA15 | PORT_PA20 | PORT_PA21 | PORT_PA06 | PORT_PA07;
    PORT->Group[0].OUTCLR.reg = PORT_PA15 | PORT_PA20 | PORT_PA21 | PORT_PA06 | PORT_PA07;
    
    // Turn on selected LED if corresponding bit in `led` bitmask is set.
    if ((led >> i) & 0x1) {
      PORT->Group[0].DIRSET.reg = DIRSETS[i];
      PORT->Group[0].OUTSET.reg = OUTSETS[i];
      PORT->Group[0].OUTCLR.reg = OUTCLRS[i];
    }
  }
}
