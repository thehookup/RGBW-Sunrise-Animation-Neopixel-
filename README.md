# RGBW-Sunrise-Animation-Neopixel-
MQTT RGBW Sunrise animation with brightness and color control via MQTT

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
    white_value_scale: 255
    retain: true
```

