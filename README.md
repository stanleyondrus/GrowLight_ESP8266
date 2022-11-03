![](doc/img/logo.png)

## Features
- Hybrid operation mode (STA & AP)
- WebSocket-based GUI
- Concurrent multi-user support
- Automatic time synchronization over NTP
- Time zone selection
- 24h on/off scheduler
- 4-channel relay switch

## Manual relay control
![](doc/img/manual.gif)

## Scheduler
![](doc/img/automatic.gif)

## Installation
This project is designed to work with [4-channel relay module](http://www.icstation.com/esp8266-wifi-channel-relay-module-remote-control-switch-wireless-transmitter-smart-home-p-13420.html), which uses an onboard MCU that controlls the relays according to the commands received from ESP-01 over UART.

![](doc/img/relaymodule.png)

## How to build your own release
### Prerequisites
- Visual Studio Code with PlatformIO extension
### Building
1. Open __*src/GrowLight*__ folder in VSCode
2. In __*src/GrowLight/src/main.cpp*__ you can customize three SSIDs and PASSWORDs for Wi-Fi STA mode:
```cpp
const char* ssid = "<your_ssid>";
const char* password = "<your_pass>";

const char* ssid1 = "<your_ssid1>";
const char* password1 = "<your_pass1>";

const char* ssid2 = "<your_ssid2>";
const char* password2 = "<your_pass2>";
```
3. In __*src/GrowLight/platformio.ini*__ set the correct *_upload_port_*, or remove to auto-detect.

4. Build and upload the filesystem
    - PlatformIO > esp01_1m > Platform > Build Filesystem Image
    - PlatformIO > esp01_1m > Platform > Upload Filesystem Image

5. Build and upload the firmware
    - PlatformIO > esp01_1m > General > Upload

## Credits
- [A Beginner's Guide to the ESP8266](https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [NTPClient](https://github.com/arduino-libraries/NTPClient)
- [AsyncElegantOTA](https://github.com/ayushsharma82/AsyncElegantOTA)
- [Time Range Wheel Slider (Circular Knob Slider)](https://github.com/jpweinerdev/timerangewheelslider)

