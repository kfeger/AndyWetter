/**The MIT License (MIT)
  See more at http://blog.squix.ch

  Entwickeltauf Arduino IDE 1.6.9 mit
  Board-Definition von https://github.com/esp8266/Arduino
  eigene Fonts machen mit http://oleddisplay.squix.ch/
*/

#include <string.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
//needed for WiFi-Manager library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <JsonListener.h>
#include "SSD1306.h"            //https://github.com/squix78/esp8266-oled-ssd1306
#include "OLEDDisplayUi.h"      //dto.
#include "Wire.h"
#include "WorldClockClient.h"   //https://github.com/skx/esp8266/blob/master/d1-weather-station/WorldClockClient.cpp
#include "WundergroundClient.h" //https://github.com/squix78/esp8266-weather-station
#include "TimeClient.h"
#include "WeatherStationFonts.h"
#include "icons.h"
#include "fonts.h"
#include "images.h"
#include <SFE_BMP180.h>       //https://github.com/sparkfun/BMP180_Breakout
#include "SparkFunHTU21D.h"   //https://github.com/sparkfun/SparkFun_HTU21D_Breakout_Arduino_Library

ADC_MODE(ADC_VCC);


/***************************
   Begin Settings
 **************************/

// Setup
const int UPDATE_INTERVAL_SECS = 10 * 60; // Update every 10 minutes

// Wunderground Settings
const boolean IS_METRIC = true;
const String WUNDERGRROUND_API_KEY = "738640203c8f39bc";  //kfeger@web.de mit PW SoPe2016
const String WUNDERGRROUND_LANGUAGE = "DL";
const String WUNDERGROUND_COUNTRY = "Germany";
const String WUNDERGROUND_CITY = "Munich";

// TimeClient settings
const float UTC_OFFSET = 2;

// Display Settings
const int I2C_DISPLAY_ADDRESS = 0x3c;
const int SDA_PIN = 0;
const int SDC_PIN = 2;

SSD1306  display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
OLEDDisplayUi ui     ( &display );

// Barometer Settings
SFE_BMP180 pressure;
#define ALTITUDE 519.0 // Höhe München lt. Wikipedia
double T = 0, P = 0, p0 = 0, a = 0;
char PBuffer[15];
char status = 0;
bool CalcDone = false, SwitchKM = false, SwitchKMdone = false;

//Feuchte Settings
HTU21D luftfeuchte;
float Humid = 0;
float LuftTemp = 0;
unsigned long LastPress = 0;

//Voltage
String VoltString;
int Volt = 0;
uint32_t lastVolt = 0;
#define VOLT_LOW_LIMIT 2350 //in mV

/***************************
   End Settings
 **************************/
String timeZoneIds [] = {"America/New_York", "Europe/London", "Europe/Berlin", "Australia/Sydney"};
WorldClockClient worldClockClient("de", "Germany", "E, dd. MMMMM yyyy", 4, timeZoneIds);


// flag changed in the ticker function every 10 minutes
bool readyForUpdate = false;

String lastUpdate = "--";

TimeClient timeClient(UTC_OFFSET);
Ticker ticker;

// Set to false, if you prefere imperial/inches, Fahrenheit
WundergroundClient wunderground(IS_METRIC);


void setReadyForWeatherUpdate() {
  Serial.println("Setting readyForUpdate to true");
  readyForUpdate = true;
}

// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
//FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4, drawCurrentWeather, drawForecast, drawPressure};
//int numberOfFrames = 7;
FrameCallback frames[] = { drawFrame2, drawFrame1, drawFrame3, drawCurrentWeather, drawForecast, drawCurrentDetails, drawPressure, drawStones, drawGoodBye};
int numberOfFrames = 9;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // initialize dispaly
  display.init();
  display.clear();
  display.display();
  // Gedaddel für einen netten Start
  int Volt = ESP.getVcc();
  Volt = (Volt / 100) * 100;
  VoltString = Volt / 1000;
  VoltString += ",";
  VoltString += Volt - (Volt / 1000) * 1000;
  VoltString.remove(3, 2);  //eine Nachkommastelle reicht
  VoltString += "V";
  if (Volt < VOLT_LOW_LIMIT)
    VoltString += " low!";
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  //String ssidShow = ssid;
  //textWidth = display.getStringWidth(ssidShow);
  display.drawString(64, 0, "LiPo-Akku");
  display.drawString(64, 30, VoltString);
  display.display();
  delay(1000);

  display.clear();
  display.setFont(URW_Chancery_L_Medium_Italic_24);
  
  display.drawString(64, 0, "Andy Mück");
  display.drawString(64, 30, "IoT-Spezial");
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.display();
  delay(2000);

  //display.flipScreenVertically();
  display.setFont(Dialog_bold_12);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);

  if (pressure.begin())
    Serial.println("BMP180 init erfolgreich");
  else
  {
    Serial.println("BMP180 init Fehler\n\n");
    delay(5000); // Pause forever.
  }
  luftfeuchte.begin();

  randomSeed(ESP.getVcc());
  int Pick = (int)random(0, 8);
  Serial.print(F("Pick = "));
  Serial.println(Pick);
  display.clear();
  switch (Pick) {
    case 0:
      display.drawXbm(DISPLAY_WIDTH / 2 - Sachsen_Width / 2, DISPLAY_HEIGHT / 2 - Sachsen_Height / 2, Sachsen_Width, Sachsen_Height, Sachsen);
      break;
    case 1:
      display.drawXbm(DISPLAY_WIDTH / 2 - Duffy_Width / 2, DISPLAY_HEIGHT / 2 - Duffy_Height / 2, Duffy_Width, Duffy_Height, Duffy);
      break;
    case 2:
      display.drawXbm(DISPLAY_WIDTH / 2 - munich_width / 2, DISPLAY_HEIGHT / 2 - munich_height / 2, munich_width, munich_height, munich);
      break;
    case 3:
      display.drawXbm(DISPLAY_WIDTH / 2 - BulbOff_Width / 2, DISPLAY_HEIGHT / 2 - BulbOff_Height / 2, BulbOff_Width, BulbOff_Height, BulbOff);
      display.display();
      delay(500);
      display.clear();
      display.drawXbm(DISPLAY_WIDTH / 2 - BulbOn_Width / 2, DISPLAY_HEIGHT / 2 - BulbOn_Height / 2, BulbOn_Width, BulbOn_Height, BulbOn);
      break;
    case 4:
      display.drawXbm(DISPLAY_WIDTH / 2 - ThumbsUp_Width / 2, DISPLAY_HEIGHT / 2 - ThumbsUp_Height / 2, ThumbsUp_Width, ThumbsUp_Height, ThumbsUp);
      break;
    case 5:
      display.drawXbm(DISPLAY_WIDTH / 2 - CheckSign_Width / 2, DISPLAY_HEIGHT / 2 - CheckSign_Height / 2, CheckSign_Width, CheckSign_Height, CheckSign);
      break;
    case 6:
      display.drawXbm(DISPLAY_WIDTH / 2 - Stones_width / 2, DISPLAY_HEIGHT / 2 - Stones_height / 2, Stones_width, Stones_height, Stones);
      break;
    case 7:
      display.drawXbm(DISPLAY_WIDTH / 2 - Loriot_Width / 2, DISPLAY_HEIGHT / 2 - Loriot_Height / 2, Loriot_Width, Loriot_Height, Loriot);
      break;
    default:
      display.drawXbm(DISPLAY_WIDTH / 2 - Snoopy_Width / 2, DISPLAY_HEIGHT / 2 - Snoopy_Height / 2, Snoopy_Width, Snoopy_Height, Snoopy);
      break;
  }
  display.display();
  delay(2500);

  display.clear();
  display.drawString(64, 16, "Versuche letztes");
  display.drawString(64, 32, "WLAN...");
  display.display();
  delay(1000);

  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();

  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect("AndyWetter")) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }


  display.clear();
  display.drawXbm(DISPLAY_WIDTH / 2 - WiFi_Logo_Width / 2, DISPLAY_HEIGHT / 2 - WiFi_Logo_Height / 2 - 5, WiFi_Logo_Width, WiFi_Logo_Height, WiFi_Logo);
  display.drawString(64, WiFi_Logo_Height + 10, WiFi.SSID());
  display.display();
  delay(2000);

  ui.setTargetFPS(30);
/*
  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);
*/
  // Add frames
  ui.disableIndicator();

  ui.setFrames(frames, numberOfFrames);

  // Inital UI takes care of initalising the display too.
  ui.init();

  Serial.println("");

  updateData(&display);

  ticker.attach(UPDATE_INTERVAL_SECS, setReadyForWeatherUpdate);

  calcPress();
  LastPress = millis();

}

void loop() {

  if (readyForUpdate && ui.getUiState()->frameState == FIXED) {
    updateData(&display);
  }

  if (ui.getUiState()->frameState == FIXED) {
    // lockaler Druck etc. alle 10sec
    if ( (millis() - LastPress) >= 10000) {
      calcPress();
      LastPress = millis();
    }
    AnimationDirection AnimationDir = (AnimationDirection)random(0, 4);
    ui.setFrameAnimation(AnimationDir);
  }

  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    //Serial.print("Rem. Time Budget: ");
    //Serial.println(remainingTimeBudget);
    delay(remainingTimeBudget);
  }
}


