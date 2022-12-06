# TinyTag: An Ultra-Low Power Bluetooth Low Energy Tracking Device

## General Description

The `TinyTag` is an ultra-low power, motion-based tracking device, similar to the [Apple AirTag](https://www.apple.com/airtag/), developed using the [TinyCircuits TinyZero](https://tinycircuits.com/products/tinyzero-processor) processor board and the [Arduino](https://www.arduino.cc/en/software) platform.

## Functional Description

The TinyTag simply detects whether it has been lost and when lost, regularly reports how long it has been lost for to nearby smartphones using [Bluetooth Low Energy (BLE)](https://www.bluetooth.com/bluetooth-resources/intro-to-bluetooth-low-energy/).

The TinyTag samples the onboard [Bosch BMA253](https://www.bosch-sensortec.com/products/motion-sensors/accelerometers/bma253/) accelerometer via I2C communication at a regular interval (default `1s`) and using a simple diff algorithm, determines if the accelerometer has been stationary between samples.

After a certain time period of being stationary (default `5m`), the TinyTag will detect that it is lost and subsequently beacon to nearby smartphones with BLE using the [TinyCircuits TinyShield](https://tinycircuits.com/products/bluetooth-low-energy-tinyshield), which is based around the [STMicroelectronics BlueNRG-MS](https://www.st.com/en/wireless-connectivity/bluenrg-ms.html) chipset.

Upon connection with a smartphone, the TinyTag will regularly (default `12s`) report that it is lost with the message "`<- TinyTag LOST ->`" along with the duration it has been lost for in the format "`DdHhMmSs`" where `D`, `H`, `M`, and `S` represent the days, hours, minutes, and seconds lost, respectively. $^1$

If the TinyTag is moved while beaconing or transmitting, it will be set to a low-power standby mode after terminating the BLE connection and setting BLE nondiscovery, if applicable, until the next time TinyTag is lost. If the TinyTag is moved while lost or the message "`<- TinyTag FOUND ->`" is received while lost and BLE transmitting, the TinyTag will no longer be lost and stop BLE transmission and discovery almost immediately.

## Power Optimizations

The TinyTag operates at a dynamic power discharge of `~6.5mW` and `~9.5mW` when found and lost, respectively, and draws `~1.5mA` and `~2.0mA` of current when found and lost, respectively. $^2$

The ultra-low power consumption is achieved through many traditional embedded system power optimizations including clock gating, CPU and peripheral sleep modes, clock source switching and disabling, lower clock frequencies, and minimizing/simplifying logic and system design.

### Footnotes

$1.$ TinyTag BLE messages can be received and sent by any nearby smartphone via a Bluetooth proximity detection app such as the [nRF Toolbox](https://www.nordicsemi.com/Products/Development-tools/nrf-toolbox) smartphone app.

$2.$ Measurements were gathered using the [AVHzY CT-3](https://store.avhzy.com/index.php?route=product/product&product_id=51) power meter during normal user operation.
