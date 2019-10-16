/*****************  START USER CONFIG SECTION *********************************/
/*****************  START USER CONFIG SECTION *********************************/
/*****************  START USER CONFIG SECTION *********************************/
/*****************  START USER CONFIG SECTION *********************************/

#define USER_SSID                 "YOUR_WIFI_SSID"
#define USER_PASSWORD             "YOUR_WIFI_PASSWORD"
#define USER_MQTT_SERVER          "YOUR MQTT SERVER"
#define USER_MQTT_PORT            1883
#define USER_MQTT_USERNAME        "YOUR_MQTT_LOGIN"
#define USER_MQTT_PASSWORD        "YOUR_MQTT_PASSWORD"
#define USER_MQTT_CLIENT_NAME     "SunriseMCU"           //used to define MQTT topics, MQTT Client ID, and ArduinoOTA
#define LWTTOPIC "LWT/" USER_MQTT_CLIENT_NAME

#if defined(ESP32)
    #define LED_PIN 23                                   //pin where the led strip is hooked up
#else
    #define LED_PIN 5                                    //pin where the led strip is hooked up
#endif
#define NUM_LEDS 150                                     //number of LEDs in the strip
#define BRIGHTNESS 255                                   //strip brightness 255 max
#define SUNSIZE 30                                       //percentage of the strip that is the "sun"


#define ENABLE_OTA 1

//#define DEBUG 1

/*****************  END USER CONFIG SECTION *********************************/
/*****************  END USER CONFIG SECTION *********************************/
/*****************  END USER CONFIG SECTION *********************************/
/*****************  END USER CONFIG SECTION *********************************/
/*****************  END USER CONFIG SECTION *********************************/
