//Wird gerufen,wenn das bisherige WLAN
//nicht mehr vorhanden ist
void configModeCallback (WiFiManager *ThisWiFiManager) {
  display.clear();
  display.drawString(64, 0, "Nicht gefunden.");
  display.drawString(64, 14, "Bitte mit WLAN");
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //Serial.println(WiFi.softAP(ssid));
  //if you used auto generated SSID, print it
  display.drawString(64, 28, ThisWiFiManager->getConfigPortalSSID());
  display.drawString(64, 42, "konfigurieren");
  display.display();
  Serial.println(ThisWiFiManager->getConfigPortalSSID());
}

void updateData(OLEDDisplay *display) {
  drawProgress(display, 25, "Zeit holen...");
  worldClockClient.updateTime();
  drawProgress(display, 50, "Wetter jetzt holen...");
  wunderground.updateConditions(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  drawProgress(display, 75, "Vorhersage holen...");
  wunderground.updateForecast(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  lastUpdate = timeClient.getFormattedTime();
  drawProgress(display, 100, "Fertig...");
  readyForUpdate = false;
  delay(1000);
}

void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(10, 28, 108, 12, percentage);
  display->display();
}

void drawClock(OLEDDisplay *display, int x, int y, int timeZoneIndex, String city, const char* icon) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 60, y + 5, city);
  display->setFont(Crushed_Plain_36);
  display->drawXbm(x, y, 60, 60, icon);
  display->drawString(x + 60, y + 15, worldClockClient.getHours(timeZoneIndex) + ":" + worldClockClient.getMinutes(timeZoneIndex));

}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  ui.disableIndicator();
  drawClock(display, x, y, 0, "New York",  new_york_bits);
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  ui.disableIndicator();
  drawClock(display, x, y, 1, "London",  london_bits);
  if(!SwitchKMdone) {
    SwitchKMdone = true;
    if(SwitchKM)
      SwitchKM = false;
    else
      SwitchKM = true;
  }
}

void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  ui.disableIndicator();
  drawClock(display, x, y, 2, "München",  munich);
  CalcDone = false; //darf nicht ober- oder unterhalb des betroffenen Frames stehen!!
  SwitchKMdone = false;
}

void drawFrame4(OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y) {
  ui.disableIndicator();
  drawClock(display, x, y, 3, "Sydney",  sydney_bits);
}

void drawCurrentDetails(OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y) {
  ui.disableIndicator();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(32 + x, 0 + y, "Feuchte");
  display->drawString(96 + x, 0 + y, "Druck");
  display->drawString(32 + x, 28 + y, "Niederschl.");
  display->drawString(96 + x, 28 + y, "heute in");

  display->setFont(ArialMT_Plain_16);
  display->drawString(32 + x, 10 + y, wunderground.getHumidity());
  display->drawString(96 + x, 10 + y, wunderground.getPressure());
  display->drawString(32 + x, 38 + y, wunderground.getPrecipitationToday());
  display->setFont(ArialMT_Plain_10);
  display->drawString(96 + x, 38 + y, "München");
}


void drawPressure (OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y) {
  // Start a pressure measurement:
  // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  ui.disableIndicator();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(32 + x, 28 + y, "Temperatur");
  display->drawString(96 + x, 0 + y, "Druck");
  display->drawString(32 + x, 0 + y, "Feuchte");
  display->drawString(96 + x, 28 + y, "hier und");

  display->setFont(ArialMT_Plain_16);
  String temp = String(LuftTemp, 1);
  temp += "°C";
  display->drawString(32 + x, 38 + y, temp);  //Temperatur
  dtostrf(p0, 10, 0, PBuffer);
  temp = PBuffer; // + " hPa rel.";
  temp.trim();
  temp += "mb";
  display->drawString(96 + x, 10 + y, temp);  //Druck
  temp = String(Humid, 0);
  temp += "%";
  display->drawString(32 + x, 10 + y, temp);  //Feuchte
  display->setFont(ArialMT_Plain_10);
  display->drawString(96 + x, 38 + y, "jetzt");

  CalcDone = true;
}

void drawHohenrhein(OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y) {
  ui.disableIndicator();
  display->setFont(Dialog_bold_12);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 16 + y, "Und heute");
  display->drawString(64 + x, 32 + y, "zuhause...");
}

void drawGoodBye(OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y) {
  //display->setFont(Luckiest_Guy_14);
  ui.disableIndicator();
  display->setFont(URW_Bookman_L_Light_Italic_16);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 0 + y, "Du wirst");
  display->drawString(64 + x, 20 + y, "uns fehlen!");
  display->drawString(64 + x, 40 + y, "AG InfoSic ITPLR");
}

void drawStones(OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y) {
  //display->setFont(Luckiest_Guy_14);
  ui.disableIndicator();
  display->drawXbm((DISPLAY_WIDTH / 2 - Stones_width / 2) + x, (DISPLAY_HEIGHT / 2 - Stones_height / 2) + y, Stones_width, Stones_height, Stones);
}

void drawCurrentWeather(OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y) {
  ui.disableIndicator();
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(60 + x, 5 + y, wunderground.getWeatherText());

  display->setFont(ArialMT_Plain_24);
  String temp = wunderground.getCurrentTemp() + "°C";
  display->drawString(56 + x, 15 + y, temp);
  int tempWidth = display->getStringWidth(temp);

  display->setFont(Meteocons_Plain_42);
  String weatherIcon = wunderground.getTodayIcon();
  int weatherIconWidth = display->getStringWidth(weatherIcon);
  display->drawString(32 + x - weatherIconWidth / 2, 05 + y, weatherIcon);
}

void drawForecastDetails(OLEDDisplay * display, int x, int y, int dayIndex) {
  ui.disableIndicator();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  String day = wunderground.getForecastTitle(dayIndex).substring(0, 3);
  day.toUpperCase();
  display->drawString(x + 20, y, day);

  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, wunderground.getForecastIcon(dayIndex));

  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, wunderground.getForecastLowTemp(dayIndex) + "|" + wunderground.getForecastHighTemp(dayIndex));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawHeaderOverlay(OLEDDisplay * display, OLEDDisplayUiState * state) {
  ui.disableIndicator();
  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  String time = timeClient.getFormattedTime().substring(0, 5);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 54, time);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  String temp = wunderground.getCurrentTemp() + "°C";
  display->drawString(128, 54, temp);
  display->drawHorizontalLine(0, 52, 128);
}

void drawForecast(OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y) {
  ui.disableIndicator();
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 2);
  drawForecastDetails(display, x + 88, y, 4);
}

void calcPress(void) {

  // Retrieve the completed pressure measurement:
  // Note that the measurement is stored in the variable P.
  // Note also that the function requires the previous temperature measurement (T).
  // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
  // Function returns 1 if successful, 0 if failure.
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      Serial.print("BH180: Temperatur ");
      Serial.print(T, 2);
      Serial.print(" C, Abs. Luftdruck: ");
    }
    status = pressure.startPressure(3);
    delay(status);
    status = pressure.getPressure(P, T);
    if (status != 0)
    {
      Serial.print(P, 2);
      Serial.print(" hPa, ");
    }
    // The pressure sensor returns abolute pressure, which varies with altitude.
    // To remove the effects of altitude, use the sealevel function and your current altitude.
    // This number is commonly used in weather reports.
    // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
    // Result: p0 = sea-level compensated pressure in mb
    p0 = pressure.sealevel(P, ALTITUDE);
    Serial.print("rel. Luftdruck: ");
    Serial.print(p0, 2);
    Serial.println(" hPa");
    Humid = luftfeuchte.readHumidity();
    LuftTemp = luftfeuchte.readTemperature();
    Serial.print("HTU21: Temperatur ");
    Serial.print(LuftTemp);
    Serial.print(" C, rel. Feuchte ");
    Serial.print(Humid);
    Serial.println(" %");
  }
}

