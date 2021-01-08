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

#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library

TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft); // Sprite class needs to be invoked
TFT_eSprite spr2 = TFT_eSprite(&tft); // Sprite class needs to be invoked

uint8_t currentScreen = 0;

void setup(void) {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);
  spr.setColorDepth(16); // 16 bit colour needed to show antialiased fonts
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  spr.loadFont(AA_FONT_SMALL);
  spr.createSprite(128, 32);
  spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK); // Set the sprite font colour and the background colour
  spr.setTextDatum(TL_DATUM);
  spr.drawString("Temperature", 0, 0 );
  spr.pushSprite(10, 10);

  spr2.loadFont(AA_FONT_LARGE);
  spr2.createSprite(200, 80);
  spr2.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  spr2.setTextDatum(TL_DATUM);
  spr2.drawString("24.3 C", 0, 0);
  spr2.pushSprite(10, 30);

  delay(5000);

  spr2.drawString("4.2 C", 0, 0);
  spr2.pushSprite(10, 30);
  delay(5000);

  // Draw changing numbers - no flicker using this plot method!

  // >>>> Note: it is best to use drawNumber() and drawFloat() for numeric values <<<<
  // >>>> this reduces digit position movement when the value changes             <<<<
  // >>>> drawNumber() and drawFloat() functions behave like drawString() and are <<<<
  // >>>> supported by setTextDatum() and setTextPadding()                        <<<<

  // for (int i = 0; i <= 200; i++) {
  //   spr.fillSprite(TFT_BLUE);
  //   spr.drawFloat(i / 100.0, 2, 50, 10); // draw with 2 decimal places at 50,10 in sprite
  //   spr.pushSprite(10, 130); // Push to TFT screen coord 10, 130
  //   delay (20);
  // }

  // spr.unloadFont(); // Remove the font to recover memory used

  // spr.deleteSprite(); // Recover memory
  
  // delay(8000);
}
