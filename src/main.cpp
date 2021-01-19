#include <Arduino.h>
#include <Button2.h>
#include <esp_adc_cal.h>
#include <Wire.h>
#include <bsec.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

const uint8_t bsec_config_iaq[] = {
  #include "config/generic_33v_3s_4d/bsec_iaq.txt"
};

#include "UI.h"
#include "Icons.h"

#define SERVICE_UUID "83613d85-967c-4a59-8190-e39097cf9a72"
#define CHARACTERISTIC_UUID "0db3cc41-1fe7-49fc-9552-84df888df33b"

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
void drawNextScreen();

void startBLE();
void notifyBLE();
BLEStatus getBLEStatus();

void initButtons();

void initBatteryVref();
BatteryLevel getBatteryLevel();
float getBatteryVoltage();

void initSensor();
boolean isSensorOk();
float temperatureCompensatedAltitude(int32_t pressure, float temp=21.0, float seaLevel=1013.25);
void serializeJsonPayload(char* payload);

Bsec sensor;
float sensorTemperature;
float calculatedAltitude;
float sensorHumidity;
float sensorPressure;
float sensorIaq;
uint8_t sensorIaqAccuracy;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
boolean bleConnected = false;
boolean wasBLEConncted = false;
boolean isBleOn = false;

float batteryVoltage = 0;

DynamicJsonDocument doc(1024);

void setup(void) {
  Serial.begin(115200);

  pinMode(PIN_ADC_EN, OUTPUT);

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

boolean isSensorOk() {
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
    batteryVoltage = getBatteryVoltage(); // update global batteryVoltage value

    boolean sensorStatus = true;
    if (sensor.run()) { // if new sensor data is available
      sensorTemperature = sensor.temperature;
      sensorHumidity = sensor.humidity;
      sensorPressure = sensor.pressure;
      sensorIaq = sensor.iaq;
      sensorIaqAccuracy = sensor.iaqAccuracy;
      calculatedAltitude = temperatureCompensatedAltitude(sensorPressure, sensorTemperature);

      drawMeasurements();
      notifyBLE();
    } else {
      sensorStatus = isSensorOk();
    }

    drawStatusBar(getBatteryLevel(), sensorStatus, getBLEStatus());
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
    break;
  case 2:
    drawIAQ(sensorIaq);
    drawIAQAccuracy(sensorIaqAccuracy);
    break;
  case 3:
    drawBatteryVoltage(batteryVoltage);
    break;
  }
}

BatteryLevel getBatteryLevel() {
  // float batteryVoltage = getBatteryVoltage();

  Serial.println(batteryVoltage);
  if(batteryVoltage >= 4.8) {
    return BAT_CHARGING;
  } else if(batteryVoltage < 4.8 && batteryVoltage > 3.6) {
    return BAT_HIGH;
  } else if(batteryVoltage >= 3.3 && batteryVoltage < 3.6) {
    return BAT_MEDIUM;
  } else if(batteryVoltage < 3.3) {
    return BAT_LOW;
  } else {
    return BAT_UNKNOWN;
  }
}

void drawNextScreen() {
  currentScreen++;
  if (currentScreen > 3) {
    currentScreen = 0;
  }
  drawMeasurements();
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
    drawStatusBar(getBatteryLevel(), isSensorOk(), getBLEStatus());
    drawMeasurements();
  });

  btnBottom.setClickHandler([](Button2 &b) {
    drawNextScreen();
  });

  btnBottom.setLongClickHandler([](Button2 &b) {
    if(!isBleOn) {
      startBLE();
    } else {
      BLEDevice::deinit(false);
    }
    isBleOn = !isBleOn;
  });
}

class BLECallback: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      bleConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      bleConnected = false;
    }
};

void startBLE() {
  BLEDevice::init("Ambient Box");
  BLEDevice::setMTU(256);
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BLECallback());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setAppearance(ESP_BLE_APPEARANCE_GENERIC_WATCH);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter

  BLEDevice::startAdvertising();
}

void notifyBLE() {
  if (bleConnected) {
    char payload[256];
    serializeJsonPayload(payload);

    pCharacteristic->setValue(payload);
    pCharacteristic->notify();
  }
  // client disconnected
  if (!bleConnected && wasBLEConncted) {
    pServer->startAdvertising(); // restart advertising
    wasBLEConncted = false;
  }
}

BLEStatus getBLEStatus() {
  if(bleConnected) {
    return BLE_CONNECTED;
  } else if(isBleOn) {
    return BLE_ON;
  } else {
    return BLE_OFF;
  }
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
  sensor.setTemperatureOffset(5.0);
}

void serializeJsonPayload(char* payload) {
  doc["temperature"] = sensorTemperature;
  doc["humidity"] = sensorHumidity;
  doc["pressure"] = sensorPressure;
  doc["altitude"] = calculatedAltitude;
  doc["iaq"] = sensorIaq;
  doc["iaqAccuracy"] = sensorIaqAccuracy;

  serializeJson(doc, payload, 256);
}

float getBatteryVoltage() {
  digitalWrite(PIN_ADC_EN, HIGH);
  delay(2); // a delay > 1 seems to be really important in order to get accurate readings...
  uint16_t measurement = analogRead(PIN_ADC);
  float vBatt = ((float) measurement / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
  digitalWrite(PIN_ADC_EN, LOW);
  return vBatt;
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