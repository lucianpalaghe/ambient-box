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
void initBatteryVref();
BatteryLevel getBatteryLevel();

float temperatureCompensatedAltitude(int32_t pressure, float temp=21.0, float seaLevel=1013.25);
void checkIaqSensorStatus(void);

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

  Wire.begin();

  sensor.begin(BME680_I2C_ADDR_PRIMARY, Wire);
  output = "\nBSEC library version " + String(sensor.version.major) + "." + String(sensor.version.minor) + "." + String(sensor.version.major_bugfix) + "." + String(sensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();

  sensor.setConfig(bsec_config_iaq);
  checkIaqSensorStatus();

  
  bsec_virtual_sensor_t sensorList[4] = {
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_IAQ,
  };

  sensor.updateSubscription(sensorList, 4, BSEC_SAMPLE_RATE_LP);
  sensor.setTemperatureOffset(1.0);
  checkIaqSensorStatus();

  output = "Timestamp [ms], temperature [°C], relative humidity [%], pressure [hPa], IAQ, IAQ accuracy";
  Serial.println(output);
}

float temperatureCompensatedAltitude(int32_t pressure, float temp, float seaLevel) {
  float altitude;
  altitude = (pow((seaLevel/((float)pressure/100.0)), (1/5.257))-1)*(temp + 273.15) / 0.0065; // Convert into altitude in meters
  return(altitude);
} 

// Helper function definitions
void checkIaqSensorStatus(void)
{
  if (sensor.status != BSEC_OK) {
    if (sensor.status < BSEC_OK) {
      output = "BSEC error code : " + String(sensor.status);
      Serial.println(output);
    } else {
      output = "BSEC warning code : " + String(sensor.status);
      Serial.println(output);
    }
  }

  if (sensor.bme680Status != BME680_OK) {
    if (sensor.bme680Status < BME680_OK) {
      output = "BME680 error code : " + String(sensor.bme680Status);
      Serial.println(output);
    } else {
      output = "BME680 warning code : " + String(sensor.bme680Status);
      Serial.println(output);
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    drawStatusBar(getBatteryLevel(), true);

    if (sensor.run()) { // If new data is available

      sensorTemperature = sensor.temperature;
      sensorHumidity = sensor.humidity;
      sensorPressure = sensor.pressure;
      sensorIaq = sensor.iaq;
      sensorIaqAccuracy = sensor.iaqAccuracy;
      calculatedAltitude = temperatureCompensatedAltitude(sensorPressure, sensorTemperature);

      unsigned long time_trigger = millis();
      output = String(time_trigger);
      output += ", " + String(sensorTemperature);
      output += ", " + String(sensorHumidity);
      output += ", " + String(sensorPressure);
      output += ", " + String(sensorIaq);
      output += ", " + String(sensorIaqAccuracy);
      Serial.println(output);

      drawMeasurements();
    } else {
      checkIaqSensorStatus();
    }
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
  digitalWrite(PIN_ADC_EN, HIGH);
  delay(1);
  float measurement = (float) analogRead(34);
  float batteryVoltage = (measurement / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
  digitalWrite(PIN_ADC_EN, LOW);
  
  // drawTemperature(batteryVoltage);
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
    drawMeasurements();
  });

  btnBottom.setPressedHandler([](Button2 &b) {
    currentScreen++;
    if (currentScreen > 1) {
      currentScreen = 0;
    }
    drawMeasurements();
  });

  // btnBottom.setLongClickHandler([](Button2 &b) {
  //  digitalWrite(PIN_ADC_EN, LOW);
  // });
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