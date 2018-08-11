/**The MIT License (MIT)
Copyright (c) 2018 by Giuliano Pisoni
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Thanks to Daniel Eichhorn (https://github.com/squix78) and LeRoy Miller (https://github.com/kd8bxp) for their inspiring code.


Base64 encoding code by Rene Nyfenegger:
http://www.adp-gmbh.ch/cpp/common/base64.html


*/

#include <M5Stack.h>
#include "fonts.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <TimeLib.h>
#include "FS.h"
#include "SPIFFS.h"

int zoom = 0;
String _BSSID = "";


int status = WL_IDLE_STATUS;
String googleKey = "";
char yourSSID[] = "";
char yourPassword[] = "";
String jsonString = "{\n";
double latitude    = 0.0;
double longitude   = 0.0;
String coordinate; // = lat,lon
String coordinatePrev; // lat=0.0&lon=0.0
String myUTC;
int myTimezone;
int count;
String name[10], craft[10],risetime[5];
float duration[5];

String base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static String encodeBase64(char* bytes_to_encode, unsigned int in_len) {
  String ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

String timeZone(String location,String timeStamp){
  Serial.println(F("TIMEZONE START"));
  String _timeZone = "";
  // Connect to HTTP server
  WiFiClientSecure client;
  client.setTimeout(10000);
  if (!client.connect("maps.googleapis.com", 443)) {
    Serial.println(F("Connection failed"));
   _timeZone = "Fail";
    return _timeZone;
  }

  Serial.println(F("Connected!"));

  // Send HTTP request
   client.println(("GET /maps/api/timezone/json?location=" + location + "&timestamp=" + timeStamp + "&key=" + googleKey + " HTTP/1.0"));
  client.println(F("Host: maps.googleapis.com"));
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    _timeZone = "Fail";
    return _timeZone;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.0 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    _timeZone = "Fail";
    return _timeZone;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    _timeZone = "Fail";
    return _timeZone;
  }

  // Allocate JsonBuffer
  // Use arduinojson.org/assistant to compute the capacity.
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);

  // Parse JSON object
  JsonObject& root = jsonBuffer.parseObject(client);
  if (!root.success()) {
    Serial.println(F("Parsing failed!"));
    _timeZone = "Fail";
    return _timeZone;
  }

  // Extract values
  Serial.println(F("Response:"));
  Serial.println(root["status"].as<char*>());
  Serial.println(root["timeZoneID"].as<char*>());
  Serial.println(root["timeZoneName"].as<char*>());
  Serial.println(root["dstOffset"].as<char*>());
  Serial.println(root["rawOffset"].as<char*>());
  String dst = root["dstOffset"].as<char*>();
  String raw = root["rawOffset"].as<char*>();
  _timeZone = String(raw.toInt() + dst.toInt());
  Serial.println(_timeZone);
  // Disconnect
  client.stop();
  return _timeZone; 
}

void geolocation(){
  Serial.println(F("GEOLOCATION START"));
 String MyCoord = "";
    //*******************************
   String multiAPString = "";
  char bssid[6];
  DynamicJsonBuffer jsonBuffer;  
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found...");
    }
   if (n > 0) {
    for (int i = 0; i < n; i++) {
      if (i > 0) {
        multiAPString += ";";
      }
      multiAPString += WiFi.BSSIDstr(i) + "," + WiFi.RSSI(i);
    }
    Serial.println(multiAPString);
    char multiAPs[multiAPString.length() + 1];
    multiAPString.toCharArray(multiAPs, multiAPString.length());
    multiAPString = encodeBase64(multiAPs, multiAPString.length());
    Serial.println(multiAPString);
    //**********************************
  // Connect to HTTP server
 WiFi.begin(yourSSID, yourPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    Serial.println("."); 
 WiFiClient client;
  client.setTimeout(10000);
  if (!client.connect("api.mylnikov.org", 80)) {
    Serial.println(("Connection failed"));
   MyCoord = "Fail";
    return;
  }

  Serial.println(("Connected!"));

  // Send HTTP request
  client.println(("GET /geolocation/wifi?v=1.1&search=" + multiAPString + " HTTP/1.1"));
  client.println(("Host: api.mylnikov.org"));
  client.println(("Connection: close"));
  if (client.println() == 0) {
    Serial.println(("Failed to send request"));
    MyCoord = "Fail";
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(("Unexpected response: "));
    Serial.println(status);
    MyCoord = "Fail";
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(("Invalid response"));
    MyCoord = "Fail";
    return;
  }

  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);

  // Parse JSON object
  JsonObject& root = jsonBuffer.parseObject(client);
  if (!root.success()) {
    Serial.println(("Parsing failed!"));
    MyCoord = "Fail";
    return;
  }

  // Extract values
  Serial.println(("Response:"));
  Serial.println(root["result"].as<char*>());
  Serial.println(root["data"]["lat"].as<char*>());
  Serial.println(root["data"]["lon"].as<char*>());
  Serial.println(root["data"]["time"].as<char*>());
  coordinate = String(root["data"]["lat"].as<char*>()) + "," + String(root["data"]["lon"].as<char*>());
  coordinatePrev = "lat=" + String(root["data"]["lat"].as<char*>()) + "&lon=" + String(root["data"]["lon"].as<char*>());
  myUTC = root["data"]["time"].as<char*>();
  myTimezone = timeZone(coordinate,myUTC).toInt();
  Serial.println("Current time : " + humanTime(myUTC.toInt()));
   // Disconnect
  client.stop();
  
  
  }
}

String ISSCoord(){
  Serial.println("ISS COORD START");
 String _ISSCoord = "";
 // Connect to HTTP server
 WiFiClient client;
  client.setTimeout(10000);
  if (!client.connect("api.open-notify.org", 80)) {
    Serial.println(F("Connection failed"));
   _ISSCoord = "Fail";
    return _ISSCoord;
  }

  Serial.println(F("Connected!"));

  // Send HTTP request
  client.println(F("GET /iss-now.json HTTP/1.0"));
  client.println(F("Host: api.open-notify.org"));
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    _ISSCoord = "Fail";
    return _ISSCoord;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    _ISSCoord = "Fail";
    return _ISSCoord;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    _ISSCoord = "Fail";
    return _ISSCoord;
  }

  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);

  // Parse JSON object
  JsonObject& root = jsonBuffer.parseObject(client);
  if (!root.success()) {
    Serial.println(F("Parsing failed!"));
    _ISSCoord = "Fail";
    return _ISSCoord;
  }

  // Extract values
  Serial.println(F("Response:"));
  Serial.println(root["message"].as<char*>());
  Serial.println(root["timestamp"].as<char*>());
  Serial.println(root["iss_position"]["latitude"].as<char*>());
  Serial.println(root["iss_position"]["longitude"].as<char*>());
  _ISSCoord = String(root["iss_position"]["latitude"].as<char*>()) + "," + String(root["iss_position"]["longitude"].as<char*>());
  Serial.println(_ISSCoord);
  // Disconnect
  client.stop();
  return _ISSCoord; 
  
}

void ISSPrevision(){
  Serial.println("ISS PREV START");
  String ISSPrev = "";
 // Connect to HTTP server
 WiFiClient client;
  client.setTimeout(10000);
  if (!client.connect("api.open-notify.org", 80)) {
    Serial.println(("Connection failed"));
   }

  Serial.println(("Connected!"));

  // Send HTTP request
  Serial.println(("GET /iss-pass.json?" + coordinatePrev + " HTTP/1.0"));
  client.println(("GET /iss-pass.json?" + coordinatePrev + " HTTP/1.0"));
  client.println(("Host: api.open-notify.org"));
  client.println(("Connection: close"));
  if (client.println() == 0) {
    Serial.println(("Failed to send request"));
    }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(("Unexpected response: "));
    Serial.println(status);
    }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(("Invalid response"));
    }

  // Allocate JsonBuffer
  // Use arduinojson.org/assistant to compute the capacity.
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);

  // Parse JSON object
  JsonObject& root = jsonBuffer.parseObject(client);
  if (!root.success()) {
    Serial.println(("Parsing failed!"));
    }
 M5.Lcd.clearDisplay();
    M5.Lcd.setBrightness(200);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setFreeFont(FF17);
    M5.Lcd.setCursor(15, 15);
    M5.Lcd.println("Next flyes over");
  // Extract values
 Serial.println(("Response:"));
  Serial.println(root["message"].as<char*>());
  Serial.println("EXTRACT START ");
  count = root["request"]["passes"];
  if (count > 5) {count = 5;}
  for (int i=0;i<count; i++){
    unsigned int riseUTC = root["response"][i]["risetime"];
    risetime[i] = humanTime(riseUTC);
    Serial.println(risetime[i]);
    duration[i] = root["response"][i]["duration"];
    duration[i] = duration[i] / 60;
    Serial.println(duration[i]);
    //M5.Lcd.drawString(risetime[i] + " for " + duration[i] + "mins" ,0 ,(i+1) * 24,GFXFF);
    M5.Lcd.println(risetime[i] + " for " + duration[i] + " mins");
      }
    
    M5.Lcd.display(); 
  // Disconnect
  client.stop();
  delay(5000);
  updateMap(zoom);
}

String humanTime(unsigned int epoch){
  Serial.println("HUMANTIME START: epoch " + String(epoch) + " + TimeShift " + String(myTimezone));
  int localTime = epoch + myTimezone;
  int h = hour(localTime);
  int m = minute(localTime);
  int d = day(localTime);
  int mn = month(localTime);
  int y = year(localTime);
   char temp[100];
  sprintf(temp, "%d/%d %d:%d ",mn,d,h,m);
  return (String)temp;
}

void setup() {
  Serial.begin(115200);
  M5.begin();
  /** add those lines if you are using ESP32 ad ILI9341 display
  M5.Lcd.setRotation(7);
  M5.Lcd.invertDisplay(false);
  **/
  M5.Lcd.setBrightness(200);
  M5.Lcd.setCursor(15, 15);
  M5.Lcd.setFreeFont(FF17);
  M5.Lcd.println("Welcome to ISS tracker on ESP32");
  M5.Lcd.println("by Giuliano Pisoni");
  delay(1000); 
  SPIFFS.begin(true);
     if(!SPIFFS.begin()){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    Serial.println();
    
    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }
 M5.Lcd.println("Searching your location ...");
 geolocation();
 M5.Lcd.println("Location found :");
 M5.Lcd.println( coordinatePrev);
 delay(1000);      
  
 WiFi.begin(yourSSID, yourPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    M5.Lcd.println("Establishing connection to WiFi..");
    }
 
  M5.Lcd.println("Connecting to your network");
  
  _BSSID = WiFi.BSSIDstr();
 Serial.println(_BSSID);
  M5.Lcd.println("UPDATING MAP ...");
  updateMap(0);

}

void downloadFile(String url, String filename) {
  Serial.println("DOWNLOADING " + url + " and saving as " + filename);

  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");

    // configure server and url
    http.begin(url);

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    if (httpCode > 0) {
      //SPIFFS.remove(filename);
      File f = SPIFFS.open(filename, "w+");
      if (!f) {
        Serial.println("file open failed");
        return;
      }
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {

        // get lenght of document (is -1 when Server sends no Content-Length header)
        int total = http.getSize();
        int len = total;
        //progressCallback(filename, 0, total);
        // create buffer for read
        uint8_t buff[2048] = { 0 };
        Serial.println("START STREAM");
        // get tcp stream
        WiFiClient * stream = http.getStreamPtr();

        // read all data from server
        while (http.connected() && (len > 0 || len == -1)) {
          // get available data size
          size_t size = stream->available();

          if (size) {
            // read up to 128 byte
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

            // write it to Serial
            f.write(buff, c);

            if (len > 0) {
              len -= c;
            }
            //progressCallback(filename, total - len, total);
          }
          delay(1);
        }
        Serial.println("STREAM DONE");
        Serial.println();
        Serial.print("[HTTP] connection closed or file end.\n");

      }
      f.close();
      Serial.print("File Closed");
    }
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}

void updateMap(int zoom){
  Serial.println("UPDATE MAP START");
  downloadFile("http://maps.googleapis.com/maps/api/staticmap?center=" + coordinate + "&zoom=" + String(zoom) + "&format=jpg-baseline&size=320x220&maptype=roadmap&markers=size:tiny%7C" + coordinate + "&markers=icon:https://images.srad.jp/topics/iss_64.png%7Cshadow:false%7C" + ISSCoord() + "&key=" + googleKey, "/staticmap.jpg");
        M5.Lcd.clearDisplay();
        M5.Lcd.setBrightness(200);
        M5.Lcd.setTextColor(0xFFFF);
        M5.Lcd.fillScreen(0x0000);
        M5.Lcd.setFreeFont(FF18);
        M5.Lcd.drawJpgFile(SPIFFS, "/staticmap.jpg");
        M5.Lcd.drawString("ZOOM -     FUNC      ZOOM +", 10, 220,GFXFF);
        M5.Lcd.display(); 
}

void loop() {
  if (M5.BtnC.wasPressed()){
     Serial.println("Button C pressed");
     zoom += 1;
     M5.Lcd.drawString("UPDATING MAP ...", 80, 100,GFXFF);
     updateMap(zoom);
  }
 
  if (M5.BtnA.wasPressed()){
    Serial.println("Button A pressed");
    if (zoom >= 1) {
      zoom -= 1;
      M5.Lcd.drawString("UPDATING MAP ...", 80, 100,GFXFF);
     updateMap(zoom); 
    }
  }

  if (M5.BtnB.wasPressed()){
     Serial.println("Button B pressed");
     M5.Lcd.drawString("UPDATING PASS TIME ...", 50, 100,GFXFF);
     ISSPrevision();
  }
  
  M5.update();
}
