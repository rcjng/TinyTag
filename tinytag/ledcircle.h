#ifndef LEDCIRCLE_H
#define LEDCIRCLE_H

// LED Circle Array constants.
const uint8_t NUMBER_OF_LEDS = 16;

const uint32_t DIRSETS[] = {PORT_PA15 | PORT_PA20, 
                            PORT_PA15 | PORT_PA20, 
                            PORT_PA15 | PORT_PA21,
                            PORT_PA15 | PORT_PA21,
                            PORT_PA20 | PORT_PA21,
                            PORT_PA20 | PORT_PA21,
                            PORT_PA20 | PORT_PA06,
                            PORT_PA20 | PORT_PA06,
                            PORT_PA15 | PORT_PA06,
                            PORT_PA15 | PORT_PA06,
                            PORT_PA21 | PORT_PA06,
                            PORT_PA21 | PORT_PA06,
                            PORT_PA21 | PORT_PA07,
                            PORT_PA21 | PORT_PA07,
                            PORT_PA06 | PORT_PA07,
                            PORT_PA06 | PORT_PA07};

const uint32_t OUTSETS[] = {PORT_PA15,
                            PORT_PA20,
                            PORT_PA15,
                            PORT_PA21,
                            PORT_PA20,
                            PORT_PA21,
                            PORT_PA20,
                            PORT_PA06,
                            PORT_PA15,
                            PORT_PA06,
                            PORT_PA06,
                            PORT_PA21,
                            PORT_PA07,
                            PORT_PA21,
                            PORT_PA07,
                            PORT_PA06};

const uint32_t OUTCLRS[] = {PORT_PA20,
                            PORT_PA15,
                            PORT_PA21,
                            PORT_PA15,
                            PORT_PA21,
                            PORT_PA20,
                            PORT_PA06,
                            PORT_PA20,
                            PORT_PA06,
                            PORT_PA15,
                            PORT_PA21,
                            PORT_PA06,
                            PORT_PA21,
                            PORT_PA07,
                            PORT_PA06,
                            PORT_PA07};

/**
 * Enable selected LEDs on the 16 LED array given a 16-bit bitmask.
 * 
 * PARAMS:
 * uint16_t led -> 16-bit bitmask for LED selection. A set bit indicates the corresponding LED is turned on, otherwise turned off. Bit 0 corresponds to LED D1, bit 1 corresponds to LED D2, and so on.
 **/
void ledcircle_select(uint16_t led);

#endif
