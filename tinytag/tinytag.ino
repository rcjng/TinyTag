#include <sam.h>

#include <SPI.h>
#include "ble.h"
#include "STBLE.h"

#include "bma253.h"
#include "ledcircle.h"
#include "timer.h"

extern "C" void __libc_init_array(void);

extern "C" int _write(int fd, const void *buf, size_t count) {
  if (fd == 1) // stdout
    SerialUSB.write((char *)buf, count);
  return 0;
}

/* === Global variables and constants === */
// Timing parameters and variables
const int INTERRUPT_PERIOD = 1000;                                            // TC3 interrupt period and acceleration data sampling period.
const int INTERRUPT_RATE = 1000 / INTERRUPT_PERIOD;                           // Number of interrupts per second given the above interrupt period.
const int NUM_SECS_PER_MIN = 60;                                              // Number of seconds in a minute.
const int NUM_SECS_PER_HOUR = 60 * NUM_SECS_PER_MIN;                          // Number of seconds in an hour.
const int NUM_SECS_PER_DAY = 24 * NUM_SECS_PER_HOUR;                          // Number of seconds in a day.

// Accelerometer/Lost detection constants
const int NUM_AXES = 3;                                                       // Number of axes of acceleration data.

const int DIFF_THRESHOLD = 21;                                                // Threshold for when a device is considered stationary based upon raw acceleration count diffs in each axis: 21 * 7.81mg/LSB ~= 164 mg
const int TOTAL_DIFF_THRESHOLD = (DIFF_THRESHOLD * 3) / 2;                    // Threshold for when a device is considered stationary based upon the total raw acceleration diffs across all axes: (1.5 * `DIFF_THRESHOLD`) * 7.81/LSB

const int SECS_LOST_THRESHOLD = 5 * NUM_SECS_PER_MIN;                         // Number of seconds stationary for device to be considered lost.
const int SAMPLES_LOST_THRESHOLD = SECS_LOST_THRESHOLD * INTERRUPT_RATE;      // Number of samples stationary for device to be considered lost.

// Accelerometer/Lost detection variables
volatile int16_t prev[NUM_AXES];                                              // Array of previous acceleration values to compare to current acceleration values.
volatile int sample = 0;                                                      // Sample counter to keep track of seconds/minutes passed when lost.
volatile int lost = 0;                                                        // Number of seconds counter to keep track of seconds passed when lost. 
volatile int stationary = 0;                                                  // Number of seconds stationary counter to keep track of the number of seconds the device has been stationary.
volatile bool isLost = false;                                                 // Whether the device is lost or not.

// BLE constants
const int LOST_MSG_PERIOD = 12;                                               // Period to send lost BLE messages.
const int BLE_MSG_LEN = 20;                                                   // Max BLE message length in bytes.
const int LOST_HEADER_MSG_LEN = 18;                                           // The lost header message length in bytes.
const uint8_t LOST_HEADER_MSG[BLE_MSG_LEN+1] = "<- TinyTag LOST ->";          // The lost header message.

// BLE variables
enum sendMsg {NONE, LOST, LOST_TIME};                                         // Enum for BLE message type (NONE: Don't send message, LOST: send initial lost message, LOST_TIME: Send lost message with duration).
volatile sendMsg msg = NONE;                                                  // The current message type to send via BLE.
/* =========================================== */

void TC3_Handler(void) {
  // Read current acceleration data
  int16_t curr[NUM_AXES];
  bma253_read_xyz(curr + X, curr + Y, curr + Z);

  // Compare current acceleration data to previous acceleration data to see if the device has been stationary or moving.
  bool isStationary = true;
  int total = 0;
  for (int i = 0; i < NUM_AXES; i++) {
    int diff = abs(curr[i] - prev[i]);
    
    total += diff;

    // If the acceleration across a single axis differs by at least `DIFF_THRESHOLD` over the last sample period, the device has moved.
    if (diff > DIFF_THRESHOLD) {
      isStationary = false;
    }

    // Store current acceleration data as previous acceleration data for handling the next sample.
    prev[i] = curr[i];
  }
  
  // Increment the number of samples stationary counter if the device remains stationary, otherwise reset to 0.
  stationary = ((total <= TOTAL_DIFF_THRESHOLD) || isStationary) ? stationary + 1 : 0;

  // Device is considered lost if it has been stationary for at least `SAMPLES_LOST_THRESHOLD` many interrupt samples.
  isLost = stationary >= SAMPLES_LOST_THRESHOLD;
  
  // Handle device is just lost.
  if (isLost) {
    stationary = (stationary > SAMPLES_LOST_THRESHOLD) ? stationary - 1 : stationary; // Prevent stationary from overflowing

    // Set `msg` to `LOST` if just lost, otherwise `LOST_TIME` if lost for a duration evenly divisible by `LOST_MSG_PERIOD`, otherwise `NONE`.
    if (lost++ == 0) {
      msg = LOST;
    } else {
      msg = (lost % LOST_MSG_PERIOD) == 0 ? LOST_TIME : NONE;      
    }
    
  // Handle device is not lost.
  } else {
    // Reset counters and message selector, device is still not lost or no longer lost.
    lost = 0;
    msg = NONE;
  }

  // Acknowledge the interrupt (clear MC0 interrupt flag to re-arm).
  TC3->COUNT16.INTFLAG.reg |= 0b00010000;
}

/**
 * Formats the `lost` number of seconds lost counter to a duration string in the form of `DAYdHOURhMINmSECs`.
 */
void format_lost_msg(char* lost_msg) {
  int sec = lost;

  int day = sec / NUM_SECS_PER_DAY;
  sec %= NUM_SECS_PER_DAY;

  int hour = sec / NUM_SECS_PER_HOUR;
  sec %= NUM_SECS_PER_HOUR;

  int min = sec / NUM_SECS_PER_MIN;
  sec %= NUM_SECS_PER_MIN;

  sprintf(lost_msg, "%ldd%ldh%ld%m%lds", day, hour, min, sec);
}

int main(void) {
  /* === Basic Initialization/Configuration === */
  init();                                                                                                           // Arduino Zero Board Initialization.
  __libc_init_array();                                                                                              // C Library Initialization.

  // Enable CPU Sleep Mode Idle Level 2
  SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);                                                                             // SLEEPDEEP disabled.                                                        
  PM->SLEEP.bit.IDLE = 0x2;                                                                                         // Idle mode 2 enabled (CPU, AHB, APB disabled).
  /* =========================================== */

  /* === Clock Configuration === */
  // Enable and configure GCLK0 and GCLK1 clocks.
  // Set GCLK0 clock to use OSC8M clock source; CPU & SERCOM3 run at 8 MHz / 4 (Prescaler) = 2 MHz
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(0) | GCLK_GENDIV_DIV(4);                                                        // Prescaler = 4. 
  while (GCLK->STATUS.bit.SYNCBUSY);
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_SRC_OSC8M | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;          // Generator source: OSC8M output
  while (GCLK->STATUS.bit.SYNCBUSY);
  // Set GCLK1 clock to use OSCULP32K clock source; TC3 runs at 32.768 kHz.
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(1);                                                                             // Prescaler = 1.
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENCTRL.reg = GCLK_GENDIV_ID(1) | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;       // Generator source: OSCULP32K output
  while (GCLK->STATUS.bit.SYNCBUSY);

  // Disable GCLK2-7 clocks.
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(2);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENCTRL.reg = GCLK_GENDIV_ID(2);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(3);
  while (GCLK->STATUS.bit.SYNCBUSY);
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(3);      
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(4);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENCTRL.reg = GCLK_GENDIV_ID(4);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(5);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENCTRL.reg = GCLK_GENDIV_ID(5);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(6);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENCTRL.reg = GCLK_GENDIV_ID(6);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(7);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENCTRL.reg = GCLK_GENDIV_ID(7);
  while (GCLK->STATUS.bit.SYNCBUSY);

  // Disable unused clock sources.
  SYSCTRL->XOSC.bit.XTALEN = 0;
  SYSCTRL->XOSC.bit.ENABLE = 0;
  SYSCTRL->XOSC32K.bit.EN32K = 0;
  SYSCTRL->XOSC32K.bit.XTALEN = 0;
  SYSCTRL->XOSC32K.bit.ENABLE = 0;
  SYSCTRL->OSC32K.bit.EN32K = 0;
  SYSCTRL->OSC32K.bit.ENABLE = 0;
  SYSCTRL->DFLLCTRL.bit.ENABLE = 0;
  SYSCTRL->DPLLCTRLA.bit.ENABLE = 0;

  // Disable unused peripheral clocks.
  PM->AHBMASK.reg   = 0x00000017;                                                                               // NVMCTRL, HPB0-2 enabled.
  PM->APBAMASK.reg  = 0x0000004E;                                                                               // EIC, GCLK, SYSCTRL, and PM enabled.
  PM->APBBMASK.reg  = 0x00000008;                                                                               // PORT enabled.
  PM->APBCMASK.reg  = 0x00000828;                                                                               // SERCOM1, SERCOM3, and TC3 enabled.
  /* =========================================== */

  /* === Driver Initialization === */
  // Initialize Bluetooth Low Energy (BLE).
  ble_init();

  // Initialize I2C communication and BMA253 accelerometer.
  i2c_init();
  bma253_init();

  // Initialize TC3 timer and set interrupts/acceleration data sampling and comparison to occur every 1000ms = 1s.
  timer3_init();
  timer3_set(INTERRUPT_PERIOD);
  timer3_reset();
  /* =========================================== */

  /* === Application Code === */
  // Initialize previous acceleration values for the first interrupt comparison.
  int16_t x_0, y_0, z_0;
  bma253_read_xyz(&x_0, &y_0, &z_0);
  prev[X] = x_0;
  prev[Y] = y_0;
  prev[Z] = z_0;

  // Configure BLE, potentially send BLE messages if lost, otherwise sleep until next the next interrupt (1s).
  while (1) {
    /**
     * Process received events, received messages, and setup/unsetup BLE for sending messages.
     * NOTE: isLost and stationary may change values after the call.
     */
    ble_process();

    // If the device is lost.
    if (isLost) {
      // Send (or do not send) a certain BLE message depending on `msg`.
      switch(msg) {
        // Send just the lost header message during the event where the device was just lost.
        case LOST:
          lib_aci_send_data(0, (uint8_t*)LOST_HEADER_MSG, LOST_HEADER_MSG_LEN);                 // Send lost header message via BLE.

          msg = NONE;
          break;
        // Send the lost header message and the duration that the device has been lost during the event where the device has been lost.
        case LOST_TIME:
          lib_aci_send_data(0, (uint8_t*)LOST_HEADER_MSG, LOST_HEADER_MSG_LEN);                 // Send lost header message via BLE.

          // Setup/format lost duration message.
          char lost_msg[BLE_MSG_LEN+1];
          int lost_msg_len;
          
          format_lost_msg(lost_msg);
          lost_msg_len = strlen(lost_msg);
          
          lib_aci_send_data(0, (uint8_t*)lost_msg, lost_msg_len);                               // Send lost duration message via BLE.

          msg = NONE;
          break;
        // Do not send a BLE message.
        case NONE:
        default:
          break;
      }
    // If the device is not lost.
    } else {
        __WFI();                                                                                // Sleep until next interrupt (1s).
    }
  }
  /* =========================================== */

  return 0;
}