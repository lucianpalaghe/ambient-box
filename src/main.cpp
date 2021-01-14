#include <Arduino.h>
#include <Button2.h>
#include "UI.h"
#include "Icons.h"
#include "esp_adc_cal.h"

#include <Wire.h>
#include "bsec.h"
const uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_3s_4d/bsec_iaq.txt"
};

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

void drawMeasurements();
void initButtons();
void drawStatusBar();
BatteryLevel getBatteryLevel();
float getBatteryVoltage();
void initBatteryVref();

void initSensor();
float temperatureCompensatedAltitude(int32_t pressure, float temp=21.0, float seaLevel=1013.25);
boolean isSensorOk();

// Create an object of the class Bsec
Bsec sensor;
String output;

float sensorTemperature;
float calculatedAltitude;
float sensorHumidity;
float sensorPressure;
float sensorIaq;
uint8_t sensorIaqAccuracy;

void setup(void) {
  Serial.begin(115200);

  pinMode(PIN_ADC, OUTPUT);
  digitalWrite(PIN_ADC, HIGH);

  initUI();
  initButtons();
  initBatteryVref();
  initSensor();
}

float temperatureCompensatedAltitude(int32_t pressure, float temp, float seaLevel) {
  float altitude;
  altitude = (pow((seaLevel/((float)pressure/100.0)), (1/5.257))-1)*(temp + 273.15) / 0.0065; // Convert into altitude in meters
  return(altitude);
} 

boolean isSensorOk()
{
  if (sensor.status != BSEC_OK || sensor.bme680Status != BME680_OK) {
    Serial.println(sensor.status);
    Serial.println(sensor.bme680Status);
    return false;
  }
  return true;
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    boolean btStatus = true;
    boolean sensorStatus = true;
    if (sensor.run()) { // if new sensor data is available
      sensorTemperature = sensor.temperature;
      sensorHumidity = sensor.humidity;
      sensorPressure = sensor.pressure;
      sensorIaq = sensor.iaq;
      sensorIaqAccuracy = sensor.iaqAccuracy;
      calculatedAltitude = temperatureCompensatedAltitude(sensorPressure, sensorTemperature);

      drawMeasurements();
    } else {
      sensorStatus = isSensorOk();
    }

    drawStatusBar(getBatteryLevel(), sensorStatus, btStatus);
  }

  btnTop.loop();
  btnBottom.loop();
}

void drawMeasurements() {
  switch (currentScreen) {
  case 0:
    drawTemperature(sensorTemperature);
    drawAltitude(calculatedAltitude);
    break;
  case 1:
    drawHumidity(sensorHumidity);
    drawPressure(sensorPressure);
  default:
    break;
  }
}

BatteryLevel getBatteryLevel() {
  float batteryVoltage = getBatteryVoltage();

  Serial.println(batteryVoltage);
  if(batteryVoltage >= 4.85) {
    return BAT_CHARGING;
  } else if(batteryVoltage >= 4.85 && batteryVoltage > 3.6) {
    return BAT_HIGH;
  } else if(batteryVoltage >= 3.3 && batteryVoltage < 3.6) {
    return BAT_MEDIUM;
  } else if(batteryVoltage < 3.3) {
    return BAT_LOW;
  }

  return BAT_UNKNOWN;
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
    drawStatusBar(getBatteryLevel(), false, true);
    drawMeasurements();
  });

  btnBottom.setPressedHandler([](Button2 &b) {
    currentScreen++;
    if (currentScreen > 1) {
      currentScreen = 0;
    }
    drawMeasurements();
  });
}

void initSensor() {
  Wire.begin();

  sensor.begin(BME680_I2C_ADDR_PRIMARY, Wire);
  sensor.setConfig(bsec_config_iaq);

  bsec_virtual_sensor_t sensorList[4] = {
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_IAQ,
  };

  sensor.updateSubscription(sensorList, 4, BSEC_SAMPLE_RATE_LP);
  sensor.setTemperatureOffset(1.0);
}

float getBatteryVoltage() {
  digitalWrite(PIN_ADC_EN, HIGH);
  delay(1);
  float measurement = (float) analogRead(34);
  float batteryVoltage = (measurement / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
  digitalWrite(PIN_ADC_EN, LOW);
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