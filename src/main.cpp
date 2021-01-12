#include <Arduino.h>

#include "fonts/NotoSansBold15.h"
#include "fonts/NotoSansBold36.h"
#include "fonts/NotoSansMonoSCB20.h"
#include "fonts/Unicode_Test_72.h"

// The font names are arrays references, thus must NOT be in quotes ""
#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36
#define AA_FONT_MONO  NotoSansMonoSCB20 // NotoSansMono-SemiCondensedBold 20pt
#define FONT_FINAL  Unicode_Test_72 // NotoSansMono-SemiCondensedBold 20pt

#define BUTTON_1        0
#define TFT_BL          4  // Display backlight control pin

#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library
#include <Button2.h>

TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft); // Sprite class needs to be invoked
TFT_eSprite spr2 = TFT_eSprite(&tft); // Sprite class needs to be invoked

Button2 btn1(BUTTON_1);

uint8_t currentScreen = 0;
boolean btnClick = false;

void drawTemperature(float temp);
void drawAltitude(uint16_t altitud);

void setup(void) {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);
  spr.setColorDepth(16); // 16 bit colour needed to show antialiased fonts
  tft.fillScreen(TFT_BLACK);

   btn1.setPressedHandler([](Button2 & b) {
     Serial.println("Click");
        btnClick = !btnClick;
        if(btnClick) {
          digitalWrite(TFT_BL, LOW);
        } else {
          digitalWrite(TFT_BL, HIGH);
        }
    });

    Serial.println("Finish init");
}

void loop() {
  for(uint16_t i = 0; i< 100;i++) {
    drawTemperature(random(1, 99) - 50 + random(1, 99) / 100.0);
    drawAltitude(i + random(1, 99));
    btn1.loop();
    delay(2000);
  }

}

void drawTemperature(float temp) {
  spr.loadFont(AA_FONT_SMALL);
  spr.createSprite(128, 32);
  spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK); // Set the sprite font colour and the background colour
  spr.setTextDatum(TL_DATUM);
  spr.drawString("Temperature", 0, 0);
  spr.pushSprite(15, 10);

  spr2.loadFont(AA_FONT_LARGE);
  spr2.createSprite(200, 80);
  spr2.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  spr2.setTextDatum(TL_DATUM);
  uint16_t xOffset = spr2.drawFloat(temp, 2, 0, 0);
  spr2.drawString("C", xOffset + 5, 0);
  spr2.pushSprite(15, 30);

  spr.unloadFont(); // Remove the font to recover memory used
  spr.deleteSprite(); // Recover memory
  spr2.unloadFont(); // Remove the font to recover memory used
  spr2.deleteSprite(); // Recover memory
}

void drawAltitude(uint16_t altitude) {
  spr.loadFont(AA_FONT_SMALL);
  spr.createSprite(128, 32);
  spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK); // Set the sprite font colour and the background colour
  spr.setTextDatum(TL_DATUM);
  spr.drawString("Altitude", 0, 0);
  spr.pushSprite(15, 74);

  spr2.loadFont(AA_FONT_LARGE);
  spr2.createSprite(200, 80);
  spr2.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  spr2.setTextDatum(TL_DATUM);
  uint16_t xOffset = spr2.drawNumber(altitude, 0, 0);
  spr2.drawString("m", xOffset + 5, 0);
  spr2.pushSprite(15, 94);

  spr.unloadFont(); // Remove the font to recover memory used
  spr.deleteSprite(); // Recover memory
  spr2.unloadFont(); // Remove the font to recover memory used
  spr2.deleteSprite(); // Recover memory
}