#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include "UI.h"
#include "Icons.h"

#include "fonts/NotoSansBold15.h"
#include "fonts/NotoSansBold36.h"
#include "fonts/IcoFontBT18.h"
#include "fonts/IcoFont36.h"

#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36
#define AA_FONT_ICONS_BT IcoFontBT18
#define AA_FONT_ICONS IcoFont36

#define SCREEN_HEIGHT 135
#define SCREEN_WIDTH 240

#define MARGIN_LEFT 15
#define MARGIN_TOP 10

void drawTitleSprite(const char *text, ScreenArea area);
void drawMeasurementSprite(const char *value, const char *unitOfMeasure, ScreenArea area);
void drawMeasurementSprite(const char *value, const char *unitOfMeasure, ScreenArea area, uint32_t textColor);
void drawMeasurementIcon(const char *value, ScreenArea area);
void drawBatteryStatus(const char *value);
void drawBatteryStatus(const char *value, uint16_t textColor);
void drawState(boolean sensorOk, BLEStatus bleStatus);
void drawStatusIcon(const char *value, uint16_t textColor, uint8_t hOffset);
BatteryLevel getBatteryLevel(float batteryVoltage);

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spriteTitle = TFT_eSprite(&tft);
TFT_eSprite spriteMeasurement = TFT_eSprite(&tft);
TFT_eSprite spriteStatus = TFT_eSprite(&tft);

uint32_t foregroundColor = TFT_LIGHTGREY;
uint32_t backgroundColor = TFT_BLACK;

void initUI() {
  tft.begin();
  tft.setRotation(1);
  // spriteTitle.setColorDepth(16);
  spriteTitle.loadFont(AA_FONT_SMALL);
  spriteTitle.createSprite(SCREEN_WIDTH - MARGIN_LEFT - 76, 32);
  // spriteMeasurement.setColorDepth(8);
  spriteMeasurement.loadFont(AA_FONT_LARGE);
  spriteMeasurement.createSprite(SCREEN_WIDTH - MARGIN_LEFT - 25, 80);
  // spriteStatus.setColorDepth(8);
  tft.fillScreen(backgroundColor);
  
}

void invertColorScheme() {
  uint32_t c = foregroundColor;
  foregroundColor = backgroundColor;
  backgroundColor = c;
  tft.fillScreen(backgroundColor);
}

void drawTemperature(float temp) {
  drawTitleSprite("Temperature", TOP);
  char tempChar[7];
  drawMeasurementSprite(dtostrf(temp, 0, 1, tempChar), GLYPH_CELSIUS, TOP);

  char tempIcon[4] = ICON_THERMOMETER;
  if(temp > 24.0) { // temperature higher than 24 C will display HOT icon
    strcpy(tempIcon, ICON_TEMP_HOT);
  } else if(temp < 3.0) { // temperature lower than 3 C will display COLD icon
    strcpy(tempIcon, ICON_TEMP_COLD);
  }

  drawMeasurementIcon(tempIcon, TOP);
}

void drawAltitude(int16_t altitude) {
  drawTitleSprite("Altitude(~)", BOTTOM);
  char altitudeChar[10];
  drawMeasurementSprite(itoa(altitude, altitudeChar, 10), "m", BOTTOM);
  drawMeasurementIcon(ICON_MOUNTAIN, BOTTOM);
}

void drawHumidity(float hum) {
  drawTitleSprite("Humidity", TOP);
  char humChar[5];
  drawMeasurementSprite(dtostrf(hum, 0, 1, humChar), "%", TOP);

  char tempIcon[4] = ICON_DROP;
  if(hum > 95) { // humidity higher than 24 C will display UMBRELLA icon
    strcpy(tempIcon, ICON_UMBRELLA);
  }
  drawMeasurementIcon(tempIcon, TOP);
}

void drawPressure(float pa) {
  drawTitleSprite("Pressure", BOTTOM);
  char pressureChar[10];
  drawMeasurementSprite(dtostrf(pa, 0, 0, pressureChar), "Pa", BOTTOM);
  drawMeasurementIcon(ICON_PRESSURE, BOTTOM);
}

void drawIAQ(float iaq) {
  drawTitleSprite("IAQ", TOP);

  if(iaq < 50) {
    drawMeasurementSprite("Great", "", TOP, TFT_GREEN);
  } else if(iaq < 100) {
    drawMeasurementSprite("Good", "", TOP);
  } else if(iaq < 200) {
    drawMeasurementSprite("Bad", "", TOP, TFT_YELLOW);
  } else {
    drawMeasurementSprite("Horrible", "", TOP, TFT_RED);
  }

  drawMeasurementIcon("", TOP);
}

void drawIAQAccuracy(uint8_t accuracy) {
  drawTitleSprite("Accuracy", BOTTOM);

  switch (accuracy)
  {
  case IAQACC_STABILIZING:
    drawMeasurementSprite("Stabilizing", "", BOTTOM);
    break;
  case IAQACC_LOW:
    drawMeasurementSprite("Low", "", BOTTOM);
    break;
  case IAQACC_MEDIUM:
    drawMeasurementSprite("Medium", "", BOTTOM);
    break;
  case IAQACC_HIGH:
    drawMeasurementSprite("High", "", BOTTOM);
    break;
  }

  // drawMeasurementIcon("", BOTTOM);
}

void drawBatteryVoltage(float vBatt) {
  drawTitleSprite("Battery", TOP);
  char battChar[5];
  drawMeasurementSprite(dtostrf(vBatt, 0, 2, battChar), "V", TOP);

  drawMeasurementSprite("", "", BOTTOM); // clear bottom screen part
}

void drawTitleSprite(const char *text, ScreenArea area) {
  uint8_t verticalPosition = 0;
  switch (area) {
  case TOP:
    verticalPosition = 0;
    break;
  case BOTTOM:
    verticalPosition = 64;
    break;
  }

  spriteTitle.fillSprite(backgroundColor);
  spriteTitle.setTextColor(foregroundColor, backgroundColor);
  spriteTitle.setTextDatum(TL_DATUM);
  spriteTitle.drawString(text, 0, 0);
  spriteTitle.pushSprite(MARGIN_LEFT, verticalPosition + MARGIN_TOP);
}

void drawMeasurementSprite(const char *value, const char *unitOfMeasure, ScreenArea area) {
  drawMeasurementSprite(value, unitOfMeasure, area, foregroundColor);
}

void drawMeasurementSprite(const char *value, const char *unitOfMeasure, ScreenArea area, uint32_t textColor) {
  uint8_t verticalPosition = 0;
  switch (area) {
  case TOP:
    verticalPosition = 20;
    break;
  case BOTTOM:
    verticalPosition = 84;
    break;
  }

  spriteMeasurement.fillSprite(backgroundColor);
  spriteMeasurement.setTextColor(textColor, backgroundColor);
  spriteMeasurement.setTextDatum(TL_DATUM);
  uint16_t uomOffset = spriteMeasurement.drawString(value, 0, 0);
  spriteMeasurement.drawString(unitOfMeasure, uomOffset + 5, 0);
  spriteMeasurement.pushSprite(MARGIN_LEFT, verticalPosition + MARGIN_TOP);
}

void drawBatteryStatus(const char *value) {
  drawBatteryStatus(value, foregroundColor);
}

void drawBatteryStatus(const char *value, uint16_t textColor) {
  spriteStatus.loadFont(AA_FONT_ICONS);
  spriteStatus.createSprite(36, 22);
  spriteStatus.fillSprite(backgroundColor);
  spriteStatus.setTextColor(textColor, backgroundColor);
  spriteStatus.setTextDatum(MC_DATUM);
  spriteStatus.drawString(value, 18, 11);
  spriteStatus.pushSprite(200, 0);
  spriteStatus.unloadFont();
  spriteStatus.deleteSprite();
}

void drawMeasurementIcon(const char *value, ScreenArea area) {
  uint8_t verticalPosition = 0;
  switch (area) {
  case TOP:
    verticalPosition = 27;
    break;
  case BOTTOM:
    verticalPosition = 95;
    break;
  }

  spriteStatus.loadFont(AA_FONT_ICONS);
  spriteStatus.createSprite(36, 36);
  spriteStatus.fillSprite(backgroundColor);
  spriteStatus.setTextColor(foregroundColor, backgroundColor);
  spriteStatus.setTextDatum(MC_DATUM);
  spriteStatus.drawString(value, 18, 16);
  spriteStatus.pushSprite(SCREEN_WIDTH - 60, verticalPosition);
  spriteStatus.unloadFont();
  spriteStatus.deleteSprite();
}

void drawState(boolean sensorOk, BLEStatus bleStatus) {
  uint8_t leftHOffset = 0;

  if(bleStatus == BLE_CONNECTED) {
    drawStatusIcon(ICON_BLUETOOTH, TFT_BLUE, 0);
    leftHOffset = 14;
  } else if(bleStatus == BLE_ON) {
    drawStatusIcon(ICON_BLUETOOTH, foregroundColor, 0);
    leftHOffset = 14;
  } else { // clear screen where the icon might have been
    drawStatusIcon("", foregroundColor, 0);
  }

  if(!sensorOk) {
    drawStatusIcon(ICON_WARNING, TFT_ORANGE, leftHOffset);
  }
}


void drawStatusIcon(const char *value, uint16_t textColor, uint8_t hOffset) {
  spriteStatus.loadFont(AA_FONT_ICONS_BT);
  spriteStatus.createSprite(32, 18);
  spriteStatus.fillSprite(backgroundColor);
  spriteStatus.setTextColor(textColor, backgroundColor);
  spriteStatus.setTextDatum(TR_DATUM);
  spriteStatus.drawString(value, 32, 0);
  spriteStatus.pushSprite(164 - hOffset, 2);
  spriteStatus.unloadFont();
  spriteStatus.deleteSprite();
}

void drawStatusBar(float batteryVoltage, boolean sensorOk, BLEStatus bleStatus) {
  drawState(sensorOk, bleStatus);

  switch (getBatteryLevel(batteryVoltage)) {
  case BAT_CHARGING:
    drawBatteryStatus(ICON_BATTRY_CHARGING, TFT_GREEN);
    break;
  case BAT_HIGH:
    drawBatteryStatus(ICON_BATTERY_HIGH);
    break;
  case BAT_MEDIUM:
    drawBatteryStatus(ICON_BATTERY_MEDIUM);
    break;
  case BAT_LOW:
    drawBatteryStatus(ICON_BATTERY_LOW, TFT_RED);
    break;
  case BAT_UNKNOWN:
    drawBatteryStatus(ICON_BATTERY_EMPTY);
    break;
  }
}

BatteryLevel getBatteryLevel(float batteryVoltage) {
  Serial.println(batteryVoltage);
  if(batteryVoltage >= 4.8) {
    return BAT_CHARGING;
  } else if(batteryVoltage < 4.8 && batteryVoltage > 3.6) {
    return BAT_HIGH;
  } else if(batteryVoltage >= 3.3 && batteryVoltage < 3.6) {
    return BAT_MEDIUM;
  } else if(batteryVoltage < 3.3) {
    return BAT_LOW;
  } else {
    return BAT_UNKNOWN;
  }
}