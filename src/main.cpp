#include <Arduino.h>
#include <Button2.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include "fonts/NotoSansBold15.h"
#include "fonts/NotoSansBold36.h"
#include "fonts/NotoSansMonoSCB20.h"
#include "fonts/Unicode_Test_72.h"

// The font names are arrays references, thus must NOT be in quotes ""
#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36

#define BUTTON_TOP 35
#define BUTTON_BOTTOM 0
#define TFT_BL 4 // Display backlight control pin

#define SCREEN_HEIGHT 135
#define SCREEN_WIDTH 240

#define MARGIN_LEFT 15
#define MARGIN_TOP 10

enum ScreenArea { TOP,
                  BOTTOM };

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);  // Sprite class needs to be invoked
TFT_eSprite spr2 = TFT_eSprite(&tft); // Sprite class needs to be invoked

Button2 btn1(BUTTON_TOP);
Button2 btn2(BUTTON_BOTTOM);

uint8_t currentScreen = 0;
boolean btnClick = false;

unsigned long previousMillis = 0;
const long interval = 2000;

void drawScreen();
void drawTemperature(float temp);
void drawAltitude(int16_t altitude);
void drawHumidity(uint8_t hum);
void drawPressure(int32_t pa);
void drawTitleSprite(const char *text, ScreenArea area);
void drawMeasurementSprite(const char *value, const char *unitOfMeasure, ScreenArea area);
void clearSpriteFonts();

void setup(void) {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);
  spr.setColorDepth(16); // 16 bit colour needed to show antialiased fonts
  tft.fillScreen(TFT_BLACK);

  btn1.setPressedHandler([](Button2 &b) {
    btnClick = !btnClick;
    if (btnClick) {
      digitalWrite(TFT_BL, LOW);
    } else {
      digitalWrite(TFT_BL, HIGH);
    }
  });

  btn2.setPressedHandler([](Button2 &b) {
    currentScreen++;
    if (currentScreen > 1) {
      currentScreen = 0;
    }
    drawScreen();
  });
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    drawScreen();
  }

  btn1.loop();
  btn2.loop();
}

void drawScreen() {
  switch (currentScreen) {
  case 0:
    drawTemperature(random(1, 99) - 50 + random(1, 99) / 100.0);
    drawAltitude(random(1, 9999));
    break;
  case 1:
    drawHumidity(random(1, 100));
    drawPressure(random(1, 9999));
  default:
    break;
  }
}

void drawTemperature(float temp) {
  drawTitleSprite("Temperature", TOP);
  char tempChar[10];
  drawMeasurementSprite(dtostrf(temp, -10, 2, tempChar), "C", TOP);
  clearSpriteFonts();
}

void drawAltitude(int16_t altitude) {
  drawTitleSprite("Altitude", BOTTOM);
  char altitudeChar[10];
  drawMeasurementSprite(itoa(altitude, altitudeChar, 10), "m", BOTTOM);
  clearSpriteFonts();
}

void drawHumidity(uint8_t hum) {
  drawTitleSprite("Humidity", TOP);
  char humChar[4];
  drawMeasurementSprite(itoa(hum, humChar, 10), "%", TOP);
  clearSpriteFonts();
}

void drawPressure(int32_t pa) {
  drawTitleSprite("Pressure", BOTTOM);
  char pressureChar[10];
  drawMeasurementSprite(itoa(pa, pressureChar, 10), "Pa", BOTTOM);
  clearSpriteFonts();
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

  spr.loadFont(AA_FONT_SMALL);
  spr.createSprite(SCREEN_WIDTH - MARGIN_LEFT, 32);
  spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK); // Set the sprite font colour and the background colour
  spr.setTextDatum(TL_DATUM);
  spr.drawString(text, 0, 0);
  spr.pushSprite(MARGIN_LEFT, verticalPosition + MARGIN_TOP);
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

  spr2.loadFont(AA_FONT_LARGE);
  spr2.createSprite(SCREEN_WIDTH - MARGIN_LEFT, 80);
  spr2.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  spr2.setTextDatum(TL_DATUM);
  uint16_t uomOffset = spr2.drawString(value, 0, 0);
  spr2.drawString(unitOfMeasure, uomOffset + 5, 0);
  spr2.pushSprite(MARGIN_LEFT, verticalPosition + MARGIN_TOP);
}

void clearSpriteFonts() {
  spr.unloadFont();
  spr.deleteSprite();
  spr2.unloadFont();
  spr2.deleteSprite();
}