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
#include "Battery.h"

#define SERVICE_UUID "83613d85-967c-4a59-8190-e39097cf9a72"
#define CHARACTERISTIC_UUID "0db3cc41-1fe7-49fc-9552-84df888df33b"

#define PIN_BUTTON_TOP 35
#define PIN_BUTTON_BOTTOM 0
#define PIN_BACKLIGHT 4
#define PIN_ADC 34
#define PIN_ADC_EN 14

Button2 btnTop(PIN_BUTTON_TOP);
Button2 btnBottom(PIN_BUTTON_BOTTOM);

uint8_t currentScreen = 0;
boolean backlightOn = true;

unsigned long previousMillis = 0;
const long interval = 2000; // screen refresh interval

void drawMeasurements();
void drawNextScreen();

void startBLE();
void notifyBLE();
BLEStatus getBLEStatus();

void initButtons();

void initSensor();
boolean isSensorOk();
float temperatureCompensatedAltitude(int32_t pressure, float temp=21.0, float seaLevel=1013.25);
void serializeJsonPayload(char* payload);
void getBytePayload(uint8_t* payload);

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

DynamicJsonDocument doc(1024);

void setup(void) {
  Serial.begin(115200);

  pinMode(PIN_ADC_EN, OUTPUT);

  initUI();
  initButtons();
  initBatteryVref();
  initSensor();

  drawLogo();
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

    drawStatusBar(getBatteryVoltage(), sensorStatus, getBLEStatus());
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
    drawBatteryVoltage(getBatteryVoltage());
    break;
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

    // redraw whole scren
    drawStatusBar(getBatteryVoltage(), isSensorOk(), getBLEStatus());
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
    // char payload[256];
    // serializeJsonPayload(payload, 256);

    uint8_t payload[20];
    getBytePayload(payload);

    pCharacteristic->setValue(payload, 20);
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


void getBytePayload(uint8_t* payload) {
  short altitude = round(calculatedAltitude);

  uint8_t* arrayTemperature = reinterpret_cast<uint8_t*>(&sensorTemperature);
  uint8_t* arrayHumidity = reinterpret_cast<uint8_t*>(&sensorHumidity);
  uint8_t* arrayPressure = reinterpret_cast<uint8_t*>(&sensorPressure);
  uint8_t* arrayAltitude = reinterpret_cast<uint8_t*>(&altitude);

  memcpy(payload, arrayTemperature, 4);
  memcpy(payload + 4, arrayHumidity, 4);
  memcpy(payload + 8, arrayPressure, 4);
  memcpy(payload + 12, arrayAltitude, 2);
  payload[14] = getIaqState(sensorIaq);
  payload[15] = sensorIaqAccuracy;
  payload[16] = getBatteryLevel(getBatteryVoltage());
}