/*****************  USER CONFIG SECTION ***************************************/
#include "config.h"

/******************  LIBRARY SECTION *************************************/
#include <NeoPixelBus.h>          //https://github.com/Makuna/NeoPixelBus
#include <SimpleTimer.h>          //https://github.com/thehookup/Simple-Timer-Library
#include <PubSubClient.h>         //https://github.com/knolleary/pubsubclient
#ifdef ESP32
  #include <WiFi.h>
  #include <ESPmDNS.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
#endif
#ifdef ENABLE_OTA
  #include <ArduinoOTA.h>           //ArduinoOTA is now included with the ArduinoIDE
#endif

#ifdef ESP32
  #define LED_ON HIGH
  #define LED_OFF LOW
#else
  #define LED_ON LOW
  #define LED_OFF HIGH
#endif

// Function definitions
void increaseSunFadeStep();
void increaseFadeStep();
void increaseWhiteLevel();
void increaseSunPhase();

/***********************  WIFI AND MQTT SETUP *****************************/
/***********************  DON'T CHANGE THIS INFO *****************************/

const char* ssid = USER_SSID ;
const char* password = USER_PASSWORD ;
const char* mqtt_server = USER_MQTT_SERVER ;
const int mqtt_port = USER_MQTT_PORT ;
const char *mqtt_user = USER_MQTT_USERNAME ;
const char *mqtt_pass = USER_MQTT_PASSWORD ;
const char *mqtt_client_name = USER_MQTT_CLIENT_NAME ;

/*****************  ENUMS         ****************************************/
enum Effects { eOff, eSunrise, eMqttRGB };

/*****************  DECLARATIONS  ****************************************/
WiFiClient espClient;
PubSubClient client(espClient);
SimpleTimer timer;
NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> strip(NUM_LEDS, LED_PIN);

/*****************  GENERAL VARIABLES  *************************************/

bool boot = true;
int sun = (SUNSIZE * NUM_LEDS)/100;
int aurora = NUM_LEDS;
//int aurora = (SUNSIZE+40 * NUM_LEDS)/100;
int sunPhase = 100;
int whiteLevel = 100;
byte red = 127; 
byte green = 127;
byte blue = 127;
byte white = 127;
Effects effect = eOff;
char charPayload[50];
int wakeDelay = 1000;
int fadeStep = 98;
int oldFadeStep = 0;
int currentAurora = 100;
int oldAurora = 0;
int currentSun = 100;
int oldSun = 0;
int sunFadeStep = 98;

#ifdef DEBUG
  unsigned int debugTime;
#endif

void setup_wifi()
{
  #ifdef ESP32
  WiFi.setSleep(false);
  #else
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  #endif
  
  WiFi.mode(WIFI_STA);

  #ifdef ESP32
    WiFi.setHostname(USER_MQTT_CLIENT_NAME);
  #else
    WiFi.hostname(USER_MQTT_CLIENT_NAME);
  #endif

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.hostname(USER_MQTT_CLIENT_NAME);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  // Loop until we're reconnected
  int retries = 0;
  while (!client.connected()) {
    if(retries < 150)
    {
        Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass))
      {
        Serial.println("connected");
        // Once connected, publish an announcement...
        if(boot == true)
        {
          client.publish(USER_MQTT_CLIENT_NAME"/checkIn","Rebooted");
          boot = false;
        }
        if(boot == false)
        {
          client.publish(USER_MQTT_CLIENT_NAME"/checkIn","Reconnected");
        }
        client.subscribe(USER_MQTT_CLIENT_NAME"/command");
        client.subscribe(USER_MQTT_CLIENT_NAME"/effect");
        client.subscribe(USER_MQTT_CLIENT_NAME"/color");
        client.subscribe(USER_MQTT_CLIENT_NAME"/white");
        client.subscribe(USER_MQTT_CLIENT_NAME"/wakeAlarm");
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        retries++;
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
    else
    {
      ESP.restart();
    }
  }
}

/************************** MQTT CALLBACK ***********************/
/************************** MQTT CALLBACK ***********************/
/************************** MQTT CALLBACK ***********************/
/************************** MQTT CALLBACK ***********************/
/************************** MQTT CALLBACK ***********************/


void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  String newTopic = topic;
  Serial.print(topic);
  Serial.print("] ");
  memset(&charPayload,0,sizeof(charPayload));
  memcpy(charPayload, payload, min(sizeof(charPayload),length));
  charPayload[length] = '\0';
  String newPayload = String(charPayload);
  int intPayload = newPayload.toInt();
  Serial.println(newPayload);
  Serial.println();
  newPayload.toCharArray(charPayload, newPayload.length() + 1);

  if (newTopic == USER_MQTT_CLIENT_NAME"/command")
  {
    //effect = newPayload;
    if ( strcmp(charPayload, "off") == 0 )
    {
      effect = eOff;
    } else if ( strcmp(charPayload, "mqttRGB") == 0 )
    {
      effect = eMqttRGB;
    }
    client.publish(USER_MQTT_CLIENT_NAME"/state", charPayload);
  }
  if (newTopic == USER_MQTT_CLIENT_NAME"/wakeAlarm")
  {
    whiteLevel = 0;
    sunPhase = 0;
    fadeStep = 0;
    sunFadeStep = 0;
    effect = eSunrise;
    wakeDelay = intPayload*10;
    timer.setTimeout(wakeDelay, increaseSunPhase);
    timer.setTimeout(wakeDelay, increaseWhiteLevel);
    timer.setTimeout((wakeDelay/80), increaseFadeStep);
    timer.setTimeout((wakeDelay/80), increaseSunFadeStep);
    client.publish(USER_MQTT_CLIENT_NAME"/command", "sunrise", true); //this sets a retained value to restore the sunrise in case of reconnect
    client.publish(USER_MQTT_CLIENT_NAME"/state", "mqttRGB"); //this is needed for the state in home assistant
  }
  if (newTopic == USER_MQTT_CLIENT_NAME"/white")
  {
    white = intPayload;
    client.publish(USER_MQTT_CLIENT_NAME"/whiteState", charPayload);
  }
  if (newTopic == USER_MQTT_CLIENT_NAME "/color")
  {
    client.publish(USER_MQTT_CLIENT_NAME "/colorState", charPayload);
    // get the position of the first and second commas
    int firstIndex = newPayload.indexOf(',');
    int lastIndex = newPayload.lastIndexOf(',');

    if ( ( firstIndex > -1) && (lastIndex > -1) && (firstIndex != lastIndex) )
    {
      uint8_t rgb_red = newPayload.substring(0, firstIndex).toInt();
      red = rgb_red;

      uint8_t rgb_green = newPayload.substring(firstIndex + 1, lastIndex).toInt();
      green = rgb_green;

      uint8_t rgb_blue = newPayload.substring(lastIndex + 1).toInt();
      blue = rgb_blue;
    }
  }
}

/*****************  GLOBAL LIGHT FUNCTIONS  *******************************/
/*****************  GLOBAL LIGHT FUNCTIONS  *******************************/
/*****************  GLOBAL LIGHT FUNCTIONS  *******************************/
/*****************  GLOBAL LIGHT FUNCTIONS  *******************************/
/*****************  GLOBAL LIGHT FUNCTIONS  *******************************/


void increaseSunPhase()
{
  if (sunPhase < 100)
  {
    sunPhase++;
    timer.setTimeout(wakeDelay, increaseSunPhase);
    Serial.print("sunPhase: ");
    Serial.println(sunPhase);
    timer.setTimeout((wakeDelay/80), increaseFadeStep);
    timer.setTimeout((wakeDelay/80), increaseSunFadeStep);
  }
}

void increaseSunFadeStep()
{
  if (sunFadeStep < 98)
  {
    sunFadeStep++;
    timer.setTimeout((wakeDelay/80), increaseSunFadeStep);
  }
}

void increaseFadeStep()
{
  if (fadeStep < 98)
  {
    fadeStep++;
    timer.setTimeout((wakeDelay/80), increaseFadeStep);
  }
}

void increaseWhiteLevel()
{
  if(whiteLevel < 100)
  {
    whiteLevel++;
    timer.setTimeout(wakeDelay, increaseWhiteLevel);
  }
}

void drawSun()
{
  currentSun = map(sunPhase, 0, 100, 0, sun);
  if(currentSun % 2 != 0)
  {
    currentSun--;
  }
  if (currentSun != oldSun)
  {
    sunFadeStep = 0;
  }

  int sunStart = (NUM_LEDS/2)-(currentSun/2);
  int newSunLeft = sunStart-1;
  int newSunRight = sunStart+currentSun;
  if(newSunLeft >= 0 && newSunRight <= NUM_LEDS && sunPhase > 0)
  {
   int redValue =  map(sunFadeStep, 0, 100, 127, 255);
   int whiteValue = map(sunFadeStep, 0, 100, 0, whiteLevel);
   strip.SetPixelColor(newSunLeft, RgbwColor(redValue, 25,0,whiteValue));
   strip.SetPixelColor(newSunRight, RgbwColor(redValue, 25,0,whiteValue));
  }
  for(int i = sunStart; i < sunStart+currentSun; i++)
  {
    strip.SetPixelColor(i, RgbwColor(255, 64,0,whiteLevel));
  }
  oldSun = currentSun;
}


void drawAurora()
{
  currentAurora = map(sunPhase, 0, 100, 0, aurora);
  if(currentAurora % 2 != 0)
  {
    currentAurora--;
  }
  if (currentAurora != oldAurora)
  {
    fadeStep = 0;
  }
  int sunStart = (NUM_LEDS/2)-(currentAurora/2);
  int newAuroraLeft = sunStart-1;
  int newAuroraRight = sunStart+currentAurora;
  if(newAuroraLeft >= 0 && newAuroraRight <= NUM_LEDS)
  {
   int redValue =  map(fadeStep, 0, 100, whiteLevel, 127);
   int greenValue =  map(fadeStep, 0, 100, 0, 25);
   strip.SetPixelColor(newAuroraRight, RgbwColor(redValue, greenValue,0,0));
   strip.SetPixelColor(newAuroraLeft, RgbwColor(redValue, greenValue,0,0));
  }
  for(int i = sunStart; i < sunStart+currentAurora; i++)
  {
    strip.SetPixelColor(i, RgbwColor(127, 25,0,0));
  }
  oldFadeStep = fadeStep;
  oldAurora = currentAurora;
}

void drawAmbient()
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    strip.SetPixelColor(i, RgbwColor(whiteLevel,0,0,0));
  }
}

void sunRise()
{
  drawAmbient();
  drawAurora();
  drawSun();
}

void off()
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    strip.SetPixelColor(i, RgbwColor(0,0,0,0)); 
  }
}

void mqttRGB()
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    strip.SetPixelColor(i, RgbwColor(red,green,blue,white)); 
  }
}

void selectEffect()
{
  switch(effect)
  {
    case eSunrise:
      sunRise();
      digitalWrite(LED_BUILTIN, LED_ON);
      break;
    case eMqttRGB:
      mqttRGB();
      digitalWrite(LED_BUILTIN, LED_ON);
      break;
    default:
      off();
      digitalWrite(LED_BUILTIN, LED_OFF);
      break;
  }
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);          // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LED_OFF);    // Turn the LED off by making the voltage HIGH
  Serial.begin(115200);
#if defined(DEBUG) && defined(ESP8266)
  	gdbstub_init();
#endif
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  #ifdef ENABLE_OTA
    ArduinoOTA.setHostname(USER_MQTT_CLIENT_NAME);
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_SPIFFS
        type = "filesystem";
      }
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });

    ArduinoOTA.begin();
  #endif

  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();
}

void loop() 
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  #ifdef ENABLE_OTA
    ArduinoOTA.handle();
  #endif
  timer.run();
  selectEffect();
  strip.Show();

  #ifdef DEBUG
    if ( millis() - debugTime > 5000 ) {
      Serial.print("FreeHeap: ");
      Serial.print(ESP.getFreeHeap());
      #if defined(ESP8266)
        Serial.print(" HeapFragmentation: ");
        Serial.print(ESP.getHeapFragmentation());
        Serial.print(" MaxFreeBlockSize: ");
        Serial.println(ESP.getMaxFreeBlockSize());
      #elif defined(ESP32)
        Serial.print(" MaxAllocHeap: ");
        Serial.println(ESP.getMaxAllocHeap());
      #endif
      debugTime = millis();
    }
  #endif
}


