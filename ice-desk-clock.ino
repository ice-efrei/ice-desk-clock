#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "bitmaps.h"
#include <PNGdec.h>
#include "ice.h" // Image is stored here in an 8 bit array
#include <time.h>
 
//ESP82266 Board Manager - https://arduino.esp8266.com/stable/package_esp8266com_index.json

// WIFI INFORMATION
#include "secrets.h"
#define JSON_MEMORY_BUFFER 1024*2

// DISPLAY PINS
#define TFT_CS 15
#define TFT_DC 5  
#define TFT_RST 4

// You can get API KEY and HOST KEY from RapidAPI, Search weatherapi.com and subscribe.
const char* API_KEY = "97fb4fcb8amshb59a3e918e8ce97p1d6b86jsn02453a7cb5a4";
const char* API_HOST = "weatherapi-com.p.rapidapi.com";

// Display and WiFiUdp 
//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
TFT_eSPI tft = TFT_eSPI(240, 240);

// 	CET-1CEST,M3.5.0,M10.5.0/3

void setTimezone(String timezone){
  Serial.printf("  Setting Timezone to %s\n",timezone.c_str());
  setenv("TZ",timezone.c_str(),1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

void initTime(String timezone){
  struct tm timeinfo;

  Serial.println("Setting up time");
  configTime(0, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
  if(!getLocalTime(&timeinfo)){
    Serial.println("  Failed to obtain time");
    return;
  }
  Serial.println("  Got the time from NTP");
  // Now we can set the real timezone
  setTimezone(timezone);
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time 1");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");
}

void  startWifi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Wifi RSSI=");
  Serial.println(WiFi.RSSI());
}

void setTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst){
  struct tm tm;

  tm.tm_year = yr - 1900;   // Set date
  tm.tm_mon = month-1;
  tm.tm_mday = mday;
  tm.tm_hour = hr;      // Set time
  tm.tm_min = minute;
  tm.tm_sec = sec;
  tm.tm_isdst = isDst;  // 1 or 0
  time_t t = mktime(&tm);
  Serial.printf("Setting time: %s", asctime(&tm));
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
}

// Latitude and Longitude of you location. 
float lat = 48.86;
float lon = 2.33;
 
// API endpoint.
String weather_url = "https://weatherapi-com.p.rapidapi.com/current.json?q=" + String(lat) + "%2C" + String(lon);

// Global variables
String current_time;
String hour;
String minute;
String alternative;
String weekDay;
String month;
int day;
int year;
int temp;

// Array for days and months
String weekDays[7]={"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
String months[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// For delay in fetching weather data.
unsigned long lastTime = 0;
unsigned long fetch_delay = 5000;

// Boot screen

PNG png; // PNG decoder inatance

#define MAX_IMAGE_WIDTH 240 // Adjust for your images

int16_t xpos = 0;
int16_t ypos = 0;

void display_boot_screen()
{
  int16_t rc = png.openFLASH((uint8_t *)ice, sizeof(ice), pngDraw);
  if (rc == PNG_SUCCESS) {
    Serial.println("Successfully opened png file");
    Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    tft.startWrite();
    uint32_t dt = millis();
    rc = png.decode(NULL, 0);
    Serial.print(millis() - dt); Serial.println("ms");
    tft.endWrite();
    // png.close(); // not needed for memory->memory decode
  }
  delay(3000);
}

void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

// https://github.com/Bodmer/TFT_eSPI/issues/142

void setup(void){

  // Initialization
  Serial.begin(9600); 
  tft.init();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Set display rotation
  tft.setRotation(1);

  // Clear display
  tft.fillScreen(0);

  // Set text color
  tft.setTextColor(TFT_CYAN);

  // Set font size
  tft.setTextSize(2);

  String loading = ".";

  // While connecting to wifi 
  while(WiFi.status() != WL_CONNECTED){  
    tft.setCursor(40, 90);
    tft.println("Connecting to ");
    tft.setCursor(40, 125);
    tft.print(WIFI_SSID);
    tft.println(loading);
    loading += ".";
    delay(500);
  }
  // Clear display
  tft.fillScreen(0);

  // Show connected
  tft.setCursor(60, 110);
  tft.println("Connected!");
  delay(3000);

  display_boot_screen();

  initTime("CET-1CEST,M3.5.0,M10.5.0/3");   // Set for Paris/FR
  printLocalTime();

  // Clear display and fetch tempurature
  tft.fillScreen(0);
  fetchTemp();
}

void loop(){
  // Fetching weather after delay
  if((millis() - lastTime) > fetch_delay){
    currentTime();
    fetchTemp();
    lastTime = millis();
  }

  // Displaying items.
  display();
}

void display(){
  // default font size = 6x8px
  int font_w = 6;
  int font_h = 8;

  // UI size
  int time_size = 6;
  int alt_size = 2;
  int day_size = 3;

  // Display WxH
  int display_w = 240;
  // int display_h = 240;

  // Distance between items
  int padding = 8;

  tft.setTextSize(time_size); // ie. 6x8 * 5 = 30x40
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // X and Y of time on screen
  int time_x = (display_w/2) - ((font_w*time_size)*5)/2 - (font_w * alt_size);
  int time_y = 40;

  tft.setCursor(time_x, time_y);
  tft.println(current_time);
  tft.setTextSize(alt_size);
  tft.setCursor((time_x + (font_w*time_size)*5), time_y);
  tft.println(alternative);
  tft.drawBitmap((time_x + (font_w*time_size)*4 + 14), (time_y + (font_h*time_size) + padding), wifi, 31, 24, TFT_WHITE);
  tft.setTextSize(day_size);
  tft.setCursor(20, time_y+(font_h*time_size) + padding + 10);
  tft.println(weekDay);
  tft.setCursor(20, time_y+(font_h*time_size) + (font_h*day_size) + padding * 2 + 10);
  tft.println(day);
  tft.setCursor(20 + (font_w * day_size)*2 + padding, time_y+(font_h*time_size) + (font_h*day_size) + padding * 2 + 10);
  tft.println(month);
  tft.setTextSize(4);
  tft.setCursor(20,  time_y+(font_h*time_size) + (font_h*day_size) * 2 + padding * 3 + 10);
  tft.println(year);
  int temp_x = display_w - (font_w * 4)*2 - padding - (font_w * alt_size);
  tft.setCursor(temp_x,  time_y+(font_h*time_size) + (font_h*day_size) + padding * 2 + 10);
  tft.println(temp);
  tft.setTextSize(alt_size);
  tft.setCursor(temp_x +(font_w * 4) *2 , time_y+(font_h*time_size) + (font_h*day_size) + padding * 2 + 10);
  tft.println("o");
  tft.setTextSize(4);
  tft.setCursor(temp_x + 10 ,time_y+(font_h*time_size) + (font_h*day_size) * 2 + padding * 3 + 10);
  tft.println("C");
}

// Formatting and setting time
void currentTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time 2");
    return;
  }

  // Formatting time
  hour = String(timeinfo.tm_hour);
  minute = String(timeinfo.tm_min);
  weekDay = weekDays[timeinfo.tm_wday];
  day = timeinfo.tm_mday;
  month = months[timeinfo.tm_mon];
  year = timeinfo.tm_year + 1900;

  Serial.println("Current time: " + hour + ":" + minute);
  Serial.println("Current date: " + weekDay + ", " + day + " " + month + " " + year);

  if(hour.toInt() < 10){
    hour = "0" + hour;
  }
  if(minute.toInt() < 10){
    minute = "0" + minute;
  }
  current_time = String(hour) + ":" + minute;
}

// Getting tempurature from API using Https request
void fetchTemp(){
  WiFiClientSecure client;
  HTTPClient https;
  client.setInsecure();
  https.useHTTP10(true);
  if(https.begin(client, weather_url.c_str())){
    https.addHeader("x-rapidapi-key", API_KEY);
    https.addHeader("x-rapidapi-host", API_HOST);

    int httpCode = https.GET();
    if(httpCode > 0){
      if(httpCode == 200){
        DynamicJsonDocument doc(JSON_MEMORY_BUFFER);
        DeserializationError error = deserializeJson(doc, https.getStream());
        Serial.print(https.getStream());
        if(error){
          Serial.println("deserialization error");
          Serial.println(error.f_str());
          temp = -1;
        }else{
          temp = doc["current"]["temp_c"].as<int>();
        }
      }
    }
  }
  https.end();
}
