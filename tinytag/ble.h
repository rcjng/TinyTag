#ifndef BLE_H
#define BLE_H

/**
 * Configures and initializes the Bluetooth Low Energy (BLE) driver.
 */
void ble_init();

/**
 * Processes Bluetooth Low Energy (BLE) events.
 * Configures BLE for discovery, connection, and operating mode depending on if the device is lost or found.
 **/
void ble_process();

uint8_t lib_aci_send_data(uint8_t ignore, uint8_t* sendBuffer, uint8_t sendLength);

#endif
