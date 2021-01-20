#include <Arduino.h>
#include <esp_adc_cal.h>
#define PIN_ADC 34
#define PIN_ADC_EN 14

int vref = 1100;
unsigned long previousMillisHW = 0;
const long intervalHW = 1000;
float batteryVoltage = 0;

float getBatteryVoltage() {
  unsigned long currentMillis = millis();

  // first function entry case
  if(currentMillis < intervalHW) {
    currentMillis = intervalHW;
  }

  if (currentMillis - previousMillisHW >= intervalHW) {
    previousMillisHW = currentMillis;

    digitalWrite(PIN_ADC_EN, HIGH);
    delay(2); // a delay > 1 seems to be really important in order to get accurate readings...
    uint16_t measurement = analogRead(PIN_ADC);
    float vBatt = ((float) measurement / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    digitalWrite(PIN_ADC_EN, LOW);
    batteryVoltage = vBatt;
  }

  return batteryVoltage;
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