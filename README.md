# TinyTag: A Low-Power Bluetooth Low Energy Tracking Device

## General Description

The `TinyTag` is a low-power, motion-based tracking device, similar to the Apple AirTag, developed using the [TinyCircuits TinyZero processor board](https://tinycircuits.com/products/tinyzero-processor) and the [Arduino platform](https://www.arduino.cc/en/software).

## Functional Description

The `TinyTag` simply detects whether it has been lost and when lost, regularly reports how long it has been lost for to nearby smartphones using Bluetooth Low Energy (BLE).

The `TinyTag` samples the onboard [Bosch BMA253 accelerometer](https://www.bosch-sensortec.com/products/motion-sensors/accelerometers/bma253/) via I2C communication at a regular interval (default 1s) and using a simple diff algorithm, determines if the accelerometer has been stationary between samples. After a certain time period of being stationary (default 5 minutes), the `TinyTag` will beacon to nearby smartphones with BLE using the [TinyShield BLE Shield based around the STMicroelectronics BlueNRG-MS chipset](https://tinycircuits.com/products/bluetooth-low-energy-tinyshield). Upon connection with a smartphone, the `TinyTag` will report that it is lost along with the duration it has been lost for. If the `TinyTag` is moved while beaconing or transmitting, it will be set to a low-power standby mode after terminating the BLE connection and setting BLE nondiscovery, if applicable, until the next time `TinyTag` is lost. If the `TinyTag` is moved while lost or the message "`<- TinyTag FOUND ->`" is received while lost and BLE transmitting, the `TinyTag` will no longer be lost and stop BLE transmission and discovery.

`TinyTag` BLE alerts are sent regularly (default every 12 seconds) and can be detected by any nearby smartphone via a Bluetooth proximity detection app such as the [nRF Toolbox smartphone app](https://www.nordicsemi.com/Products/Development-tools/nrf-toolbox).

Power optimization features such as clock gating, CPU  and peripheral sleep modes, clock source disabling, lower clock frequencies, and minimal logic are included to achieve the best balance between performance and battery life.
