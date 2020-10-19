# TeamAT_Sensor_ArduinoLib
TeamAT sensor app Arduino Library for ESP32
Alpha version
___
## Arduino Library usage
### This library is developed for ESP32 (ESP-WROOM-32)

![Esp32 pinout](https://raw.githubusercontent.com/espressif/arduino-esp32/master/docs/esp32_pinmap.png) (source: espressif arduino-esp32)

### Dependencies
- https://github.com/espressif/arduino-esp32 

### First steps
1. Follow espressif's instructions to setup your esp32. (see **Dependencies**)
2. Download this project to the Arduino library directory on your computer (*C:\Users\user\Documents\Arduino\libraries* on Window)
3. You can try the provided example *loggerExample.ino*
4. In your *.ino* file you will have to write your wifi SSID and password, your account's credentials and the id of your devices to the indicated locations.
5. Upload the code. Your device sould now log data to the server. 

___
## Web app usage

### First steps
1. Create an account or login.
2. Create a device. [*Devices -> Manage -> Add Device*]
3. Note your device ID, it's useful with the arduino library
4. You can see all your data in the *Overview* panel [*Devices -> Overview*]
5. You cans see the data and options for a specific device by clicking on its name in the *Device* menu.

___
