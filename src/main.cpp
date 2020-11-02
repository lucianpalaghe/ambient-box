#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Button2.h>
#define ADC_EN 14 //ADC_EN is the ADC detection enable port

TFT_eSPI tft = TFT_eSPI();

void espDelay(int ms);

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
    */
  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(0, 0);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);

  if (TFT_BL > 0)
  {                                         // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    pinMode(TFT_BL, OUTPUT);                // Set backlight pin to output mode
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
  }

  espDelay(5000);

  tft.setRotation(0);
  tft.fillScreen(TFT_RED);
  espDelay(1000);
  tft.fillScreen(TFT_BLUE);
  espDelay(1000);
  tft.fillScreen(TFT_GREEN);
  espDelay(1000);

  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  tft.drawString("LeftButton:", tft.width() / 2, tft.height() / 2 - 16);
  tft.drawString("[WiFi Scan]", tft.width() / 2, tft.height() / 2);
  tft.drawString("RightButton:", tft.width() / 2, tft.height() / 2 + 16);
  tft.drawString("[Voltage Monitor]", tft.width() / 2, tft.height() / 2 + 32);
  tft.drawString("RightButtonLongPress:", tft.width() / 2, tft.height() / 2 + 48);
  tft.drawString("[Deep Sleep]", tft.width() / 2, tft.height() / 2 + 64);
  tft.setTextDatum(TL_DATUM);
}

void loop()
{
  // put your main code here, to run repeatedly:
}

//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms)
{
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}