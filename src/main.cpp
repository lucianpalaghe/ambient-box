#include <Arduino.h>

#include "fonts/NotoSansBold15.h"
#include "fonts/NotoSansBold36.h"
#include "fonts/NotoSansMonoSCB20.h"

// The font names are arrays references, thus must NOT be in quotes ""
#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36
#define AA_FONT_MONO  NotoSansMonoSCB20 // NotoSansMono-SemiCondensedBold 20pt

#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library

TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft); // Sprite class needs to be invoked

uint8_t currentScreen = 0;

void setup(void) {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);
  spr.setColorDepth(16); // 16 bit colour needed to show antialiased fonts
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  // Small font
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

  spr.loadFont(AA_FONT_SMALL); // Must load the font first into the sprite class

  spr.createSprite(128, 32);   // Create a sprite 100 pixels wide and 50 high

  spr.fillSprite(TFT_BLACK);

  spr.setTextColor(TFT_YELLOW, TFT_BLACK); // Set the sprite font colour and the background colour

  spr.setTextDatum(MC_DATUM); // Middle Centre datum
  
  spr.drawString("Temperature", 50, 25 ); // Coords of middle of 100 x 50 Sprite

  spr.pushSprite(10, 10); // Push to TFT screen coord 10, 10

  spr.pushSprite(10, 70, TFT_BLUE); // Push to TFT screen, TFT_BLUE is transparent
 
  spr.unloadFont(); // Remove the font from sprite class to recover memory used

  delay(4000);

  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  // Large font
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

  tft.fillScreen(TFT_BLACK);

  // Beware: Sprites are a differerent "class" to TFT, so different fonts can be loaded
  // in the tft and sprite instances, so load the font in the class instance you use!
  // In this example this means the spr. instance.

  spr.loadFont(AA_FONT_LARGE); // Load another different font into the sprite instance

  // 100 x 50 sprite was created above and still exists...

  spr.fillSprite(TFT_GREEN);

  spr.setTextColor(TFT_BLACK, TFT_GREEN); // Set the font colour and the background colour

  spr.setTextDatum(MC_DATUM); // Middle Centre datum

  spr.drawString("Fits", 50, 25); // Make sure text fits in the Sprite!
  spr.pushSprite(10, 10);         // Push to TFT screen coord 10, 10

  spr.fillSprite(TFT_RED);
  spr.setTextColor(TFT_WHITE, TFT_RED); // Set the font colour and the background colour

  spr.drawString("Too big", 50, 25); // Text is too big to all fit in the Sprite!
  spr.pushSprite(10, 70);            // Push to TFT screen coord 10, 70

  // Draw changing numbers - no flicker using this plot method!

  // >>>> Note: it is best to use drawNumber() and drawFloat() for numeric values <<<<
  // >>>> this reduces digit position movement when the value changes             <<<<
  // >>>> drawNumber() and drawFloat() functions behave like drawString() and are <<<<
  // >>>> supported by setTextDatum() and setTextPadding()                        <<<<

  spr.setTextDatum(TC_DATUM); // Top Centre datum

  spr.setTextColor(TFT_WHITE, TFT_BLUE); // Set the font colour and the background colour

  for (int i = 0; i <= 200; i++) {
    spr.fillSprite(TFT_BLUE);
    spr.drawFloat(i / 100.0, 2, 50, 10); // draw with 2 decimal places at 50,10 in sprite
    spr.pushSprite(10, 130); // Push to TFT screen coord 10, 130
    delay (20);
  }

  spr.unloadFont(); // Remove the font to recover memory used

  spr.deleteSprite(); // Recover memory
  
  delay(8000);
}
