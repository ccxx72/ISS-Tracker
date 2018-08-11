# ISS-Tracker
## A tracker for the International Space Station on ESP32

### Thanks to:
### Daniel Eichhorn (https://github.com/squix78) 
### LeRoy Miller (https://github.com/kd8bxp) 
### Rene Nyfenegger: http://www.adp-gmbh.ch/cpp/common/base64.html 
### for their inspiring code and
### ALEXANDER MYLNIKOV https://www.mylnikov.org
### for the Geolocation API




# Hardware required
## M5Stack
  https://www.banggood.com/M5Stack-Extensible-Micro-Control-Module-WiFi-Bluetooth-ESP32-Development-Kit-For-Arduino-LCD-p-1236069.html?p=3F271674015120140854

### or (this setup requires small changes to the file M5Stack\src\utility\Display.h).
    #define TFT_WIDTH 240  // was 320
    #define TFT_HEIGHT 320  //was 240

## ESP32
  https://www.banggood.com/Wemos-Pro-ESP32-WIFI-Bluetooth-Board-4MB-Flash-p-1164459.html?p=3F271674015120140854

## 2.8 Inch ILI9341 240x320 SPI TFT LCD Display Touch Panel
  https://www.banggood.com/2_8-Inch-ILI9341-240x320-SPI-TFT-LCD-Display-Touch-Panel-SPI-Serial-Port-Module-p-1206782.html?p=3F271674015120140854
  
  
  ## Connections:
  
  ### Display - ESP32
  
  VCC - 3.3
  
  GND - GND
  
  CS - 14
  
  RESET - 33
  
  D/C - 27
  
  SDI - 23
  
  SCK - 18
  
  LED - Vin
  
  ### Buttons (not all devboard expose those pins. You can change this changing rows of M5Stack\src\utility\Config.h)
  
    #define BUTTON_A_PIN 39
    #define BUTTON_B_PIN 38
    #define BUTTON_C_PIN 37
  
# Libraries required
## M5Stack
https://github.com/m5stack/M5Stack

## ArduinoJson
https://github.com/bblanchon/ArduinoJson

## You need a Google API Key
https://developers.google.com/maps/documentation/embed/get-api-key






  
