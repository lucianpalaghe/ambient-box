
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

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spriteTitle = TFT_eSprite(&tft);
TFT_eSprite spriteMeasurement = TFT_eSprite(&tft);
TFT_eSprite spriteStatus = TFT_eSprite(&tft);

uint32_t foregroundColor = TFT_LIGHTGREY;
uint32_t backgroundColor = TFT_BLACK;

void initUI() {
  tft.begin();
  tft.setRotation(1);
  spriteTitle.setColorDepth(16);
  spriteMeasurement.setColorDepth(16);
  spriteStatus.setColorDepth(16);
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
  char tempChar[10];
  drawMeasurementSprite(dtostrf(temp, -10, 1, tempChar), "\u2103", TOP);

  char tempIcon[4] = ICON_THERMOMETER;
  if(temp > 24) { // temperature higher than 24 C will display HOT icon
    strcpy(tempIcon, ICON_TEMP_HOT);
  } else if(temp < 3) { // temperature lower than 3 C will display COLD icon
    strcpy(tempIcon, ICON_TEMP_COLD);
  }

  drawMeasurementIcon(tempIcon, TOP);
}

void drawAltitude(int16_t altitude) {
  drawTitleSprite("Altitude", BOTTOM);
  char altitudeChar[10];
  drawMeasurementSprite(itoa(altitude, altitudeChar, 10), "m", BOTTOM);
  drawMeasurementIcon(ICON_MOUNTAIN, BOTTOM);
}

void drawHumidity(float hum) {
  drawTitleSprite("Humidity", TOP);
  char humChar[5];
  drawMeasurementSprite(dtostrf(hum, -10, 1, humChar), "%", TOP);

  char tempIcon[4] = ICON_DROP;
  if(hum > 95) { // humidity higher than 24 C will display UMBRELLA icon
    strcpy(tempIcon, ICON_UMBRELLA);
  }
  drawMeasurementIcon(tempIcon, TOP);
}

void drawPressure(float pa) {
  drawTitleSprite("Pressure", BOTTOM);
  char pressureChar[10];
  drawMeasurementSprite(dtostrf(pa, -10, 0, pressureChar), "Pa", BOTTOM);
  drawMeasurementIcon(ICON_PRESSURE, BOTTOM);
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

  spriteTitle.loadFont(AA_FONT_SMALL);
  spriteTitle.createSprite(SCREEN_WIDTH - MARGIN_LEFT - 76, 32);
  spriteTitle.fillSprite(backgroundColor);
  spriteTitle.setTextColor(foregroundColor, backgroundColor);
  spriteTitle.setTextDatum(TL_DATUM);
  spriteTitle.drawString(text, 0, 0);
  spriteTitle.pushSprite(MARGIN_LEFT, verticalPosition + MARGIN_TOP);
  spriteTitle.unloadFont();
  spriteTitle.deleteSprite();
}

void drawMeasurementSprite(const char *value, const char *unitOfMeasure, ScreenArea area) {
  uint8_t verticalPosition = 0;
  switch (area) {
  case TOP:
    verticalPosition = 20;
    break;
  case BOTTOM:
    verticalPosition = 84;
    break;
  }

  spriteMeasurement.loadFont(AA_FONT_LARGE);
  spriteMeasurement.createSprite(SCREEN_WIDTH - MARGIN_LEFT - 45, 80);
  spriteMeasurement.fillSprite(backgroundColor);
  spriteMeasurement.setTextColor(foregroundColor, backgroundColor);
  spriteMeasurement.setTextDatum(TL_DATUM);
  uint16_t uomOffset = spriteMeasurement.drawString(value, 0, 0);
  spriteMeasurement.drawString(unitOfMeasure, uomOffset + 5, 0);
  spriteMeasurement.pushSprite(MARGIN_LEFT, verticalPosition + MARGIN_TOP);
  spriteMeasurement.unloadFont();
  spriteMeasurement.deleteSprite();
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

void drawState(boolean sensorState, boolean btState) {
  String state = "";
  if(!sensorState) {
    state += ICON_WARNING;
  }
  if(btState) {
    state += ICON_BLUETOOTH;
  }
  
  spriteStatus.loadFont(AA_FONT_ICONS_BT);
  spriteStatus.createSprite(32, 18);
  spriteStatus.fillSprite(backgroundColor);
  spriteStatus.setTextColor(foregroundColor, backgroundColor);
  spriteStatus.setTextDatum(TR_DATUM);
  spriteStatus.setTextSize(5);
  spriteStatus.drawString(state, 32, 0);
  spriteStatus.pushSprite(164, 2);
  spriteStatus.unloadFont();
  spriteStatus.deleteSprite();
}

void drawStatusBar(BatteryLevel battery, boolean sensorOk, boolean btConnected) {
  drawState(sensorOk, btConnected);

  switch (battery) {
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