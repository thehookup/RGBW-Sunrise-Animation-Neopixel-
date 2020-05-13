# RGBW-Sunrise-Animation-Neopixel-
MQTT RGBW Sunrise animation with brightness and color control via MQTT'

This repository is to accompany my LED Sunrise Video:

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/2j3UyrBjB4g/0.jpg)](https://www.youtube.com/watch?v=2j3UyrBjB4g)


## Initial Setup

1. Download the .ino file
2. Ensure you have the correct libraries download and installed (links at top of the ino file)
3. Update information in the user configuration section and upload to the NodeMCU
4. Hook up your NodeMCU, to your RGBW LED strip (default pin is D1 which is GPIO-5)
5. Send the number of minutes you'd like the sunrise to be spread over to the MQTT topic:

```c++
[USER_MQTT_CLIENT_NAME]/wakeAlarm
```



## Parts (Amazon Links)

NodeMCU	                	https://amzn.to/2MkBcka


RGBW WS2812B LED Strip	 	http://bit.ly/2QaujCG


Power Supply		          https://amzn.to/2TAwrG2



## Home Assistant YAML

```yaml
light:
  - platform: mqtt
    name: "Bedroom Valance"
    command_topic: "SunriseMCU/command"
    payload_on: "mqttRGB"
    payload_off: "off"
    state_topic: "SunriseMCU/state"
    rgb_command_topic: "SunriseMCU/color"
    rgb_state_topic: "SunriseMCU/colorState"
    white_value_command_topic: "SunriseMCU/white"
    white_value_state_topic: "SunriseMCU/whiteState"
    availability_topic: "LWT/SunriseMCU"
    payload_available: "Online"
    payload_not_available: "Offline"
    white_value_scale: 255
    retain: true
```

## How to build with PlatformIO

1. Install PlatformIO Core <http://docs.platformio.org/page/core.html>
2. Clone the repo with git or download the sources as .zip and extract it
3. Change into the folder where the platformio.ini is
4. Run these commands:
```shell
  # Download/Install project libraries
  > platformio lib install

  # Build project
  > platformio run

  # Upload firmware to NodeMCU 1.0
  > platformio run -e nodemcuv2 --target upload

  # Upload firmware to DOIT ESP32 DEVKIT V1
  > platformio run -e esp32doit-devkit-v1 --target upload

  # Clean build files
  > platformio run --target clean

