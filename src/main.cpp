#include <Arduino.h>
#include <Button2.h>
#include "UI.h"
#include "Icons.h"
#include "esp_adc_cal.h"

#define PIN_BUTTON_TOP 35
#define PIN_BUTTON_BOTTOM 0
#define PIN_BACKLIGHT 4
#define PIN_ADC 34
#define PIN_ADC_EN 14

int vref = 1100;

Button2 btnTop(PIN_BUTTON_TOP);
Button2 btnBottom(PIN_BUTTON_BOTTOM);

uint8_t currentScreen = 0;
boolean backlightOn = true;

unsigned long previousMillis = 0;
const long interval = 2000;

void drawScreen();
void initButtons();
void drawStatusBar();
void initBatteryVref();
BatteryLevel getBatteryLevel();

void setup(void) {
  Serial.begin(115200);

  /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
    */
    pinMode(PIN_ADC, OUTPUT);
    digitalWrite(PIN_ADC, HIGH);


  initUI();
  initButtons();
  initBatteryVref();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    drawScreen();
  }

  btnTop.loop();
  btnBottom.loop();
}

void drawScreen() {
  drawStatusBar();

  switch (currentScreen) {
  case 0:
    // drawTemperature(random(1, 99) - 50 + random(1, 99) / 100.0);
    drawAltitude(random(1, 9999));
    break;
  case 1:
    drawHumidity(random(1, 100));
    drawPressure(random(1, 9999));
  default:
    break;
  }
}

void drawStatusBar() {
  drawStatusBar(getBatteryLevel(), true);

  // switch (random(0,3))
  //   {
  //   case 0:
  //     drawStatusBar(BAT_HIGH, true);
  //     break;
  //   case 1:
  //     drawStatusBar(BAT_MEDIUM, false);
  //     break;
  //   case 2:
  //     drawStatusBar(BAT_LOW, true);
  //   default:
  //     break;
  //   }
}

BatteryLevel getBatteryLevel() {
  digitalWrite(PIN_ADC_EN, HIGH);
  delay(1);
  float measurement = (float) analogRead(34);
  float batteryVoltage = (measurement / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
  digitalWrite(PIN_ADC_EN, LOW);
  
  drawTemperature(batteryVoltage);
  Serial.println(batteryVoltage);
  if(batteryVoltage >= 3.6) {
    return BAT_HIGH;
  } else if(batteryVoltage >= 3.3 && batteryVoltage < 3.6) {
    return BAT_MEDIUM;
  } else if(batteryVoltage < 3.3) {
    return BAT_LOW;
  }
}

void initButtons() {
  btnTop.setClickHandler([](Button2 &b) {
    backlightOn = !backlightOn;
    if (backlightOn) {
      digitalWrite(PIN_BACKLIGHT, HIGH);
    } else {
      digitalWrite(PIN_BACKLIGHT, LOW);
    }
  });

  btnTop.setLongClickHandler([](Button2 &b) {
    invertColorScheme();
    drawScreen();
  });

  btnBottom.setPressedHandler([](Button2 &b) {
    currentScreen++;
    if (currentScreen > 1) {
      currentScreen = 0;
    }
    drawScreen();
  });
}

void initBatteryVref() {
  esp_adc_cal_characteristics_t adc_chars;
  // esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC_ATTEN_DB_2_5, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    vref = adc_chars.vref;
  }
  Serial.println(vref);
  pinMode(PIN_ADC_EN, OUTPUT);
}