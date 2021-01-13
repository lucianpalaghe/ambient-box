
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include "UI.h"
#include "Icons.h"

#include "fonts/NotoSansBold15.h"
#include "fonts/NotoSansBold36.h"
#include "fonts/IcoFontBT12.h"
#include "fonts/IcoFont36.h"

#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36
#define AA_FONT_ICONS_BT IcoFontBT12
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

void initUI()
{
  tft.begin();
  tft.setRotation(1);
  spriteTitle.setColorDepth(16);
  spriteMeasurement.setColorDepth(16);
  spriteStatus.setColorDepth(16);
  tft.fillScreen(backgroundColor);
  
}

void invertColorScheme()
{
  uint32_t c = foregroundColor;
  foregroundColor = backgroundColor;
  backgroundColor = c;
  tft.fillScreen(backgroundColor);
}

void drawTemperature(float temp)
{
  drawTitleSprite("Temperature", TOP);
  char tempChar[10];
  drawMeasurementSprite(dtostrf(temp, -10, 2, tempChar), "C", TOP);
}

void drawAltitude(int16_t altitude)
{
  drawTitleSprite("Altitude", BOTTOM);
  char altitudeChar[10];
  drawMeasurementSprite(itoa(altitude, altitudeChar, 10), "m", BOTTOM);
}

void drawHumidity(uint8_t hum)
{
  drawTitleSprite("Humidity", TOP);
  char humChar[4];
  drawMeasurementSprite(itoa(hum, humChar, 10), "%", TOP);
}

void drawPressure(int32_t pa)
{
  drawTitleSprite("Pressure", BOTTOM);
  char pressureChar[10];
  drawMeasurementSprite(itoa(pa, pressureChar, 10), "Pa", BOTTOM);
}

void drawTitleSprite(const char *text, ScreenArea area)
{
  uint8_t verticalPosition = 0;
  switch (area)
  {
  case TOP:
    verticalPosition = 0;
    break;
  case BOTTOM:
    verticalPosition = 64;
    break;
  }

  spriteTitle.loadFont(AA_FONT_SMALL);
  spriteTitle.createSprite(SCREEN_WIDTH - MARGIN_LEFT - 60, 32);
  spriteTitle.fillSprite(backgroundColor);
  spriteTitle.setTextColor(foregroundColor, backgroundColor);
  spriteTitle.setTextDatum(TL_DATUM);
  spriteTitle.drawString(text, 0, 0);
  spriteTitle.pushSprite(MARGIN_LEFT, verticalPosition + MARGIN_TOP);
  spriteTitle.unloadFont();
  spriteTitle.deleteSprite();
}

void drawMeasurementSprite(const char *value, const char *unitOfMeasure, ScreenArea area)
{
  uint8_t verticalPosition = 0;
  switch (area)
  {
  case TOP:
    verticalPosition = 20;
    break;
  case BOTTOM:
    verticalPosition = 84;
    break;
  }

  spriteMeasurement.loadFont(AA_FONT_LARGE);
  spriteMeasurement.createSprite(SCREEN_WIDTH - MARGIN_LEFT - 40, 80);
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

void drawBatteryStatus(const char *value, uint16_t textColor)
{
  spriteStatus.loadFont(AA_FONT_ICONS);
  spriteStatus.createSprite(36, 22);
  spriteStatus.fillSprite(backgroundColor);
  spriteStatus.setTextColor(textColor, backgroundColor);
  spriteStatus.setTextDatum(MC_DATUM);
  spriteStatus.drawString(value, 18, 12);
  spriteStatus.pushSprite(200, 0);
  spriteStatus.unloadFont();
  spriteStatus.deleteSprite();
}

void drawBT(boolean on)
{
  spriteStatus.loadFont(AA_FONT_ICONS_BT);
  spriteStatus.createSprite(16, 20);
  spriteStatus.fillSprite(backgroundColor);
  spriteStatus.setTextColor(foregroundColor, backgroundColor);
  spriteStatus.setTextDatum(TL_DATUM);
  spriteStatus.setTextSize(5);
  spriteStatus.drawString(on ? ICON_BLUETOOTH : "", 0, 0);
  spriteStatus.pushSprite(186, 2);
  spriteStatus.unloadFont();
  spriteStatus.deleteSprite();
}

void drawStatusBar(BatteryLevel battery, boolean btConnected) {
  drawBT(btConnected);

  switch (battery) {
  case BAT_HIGH:
    drawBatteryStatus(ICON_BATTERY_HIGH);
    break;
  case BAT_MEDIUM:
    drawBatteryStatus(ICON_BATTERY_MEDIUM);
    break;
  case BAT_LOW:
    drawBatteryStatus(ICON_BATTERY_LOW, TFT_RED);
    break;
  }
}