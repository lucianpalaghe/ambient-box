#include <Arduino.h>

enum ScreenArea { TOP, BOTTOM };
enum BatteryLevel { BAT_LOW, BAT_MEDIUM, BAT_HIGH, BAT_CHARGING, BAT_UNKNOWN };
enum IAQState { IAQ_HORRIBLE, IAQ_BAD, IAQ_GOOD, IAQ_GREAT };
enum IAQAccuracy { IAQACC_STABILIZING, IAQACC_LOW, IAQACC_MEDIUM, IAQACC_HIGH };
enum BLEStatus { BLE_OFF, BLE_ON, BLE_CONNECTED };

void initUI();
void invertColorScheme();
void drawTemperature(float temp);
void drawAltitude(int16_t altitude);
void drawHumidity(float hum);
void drawPressure(float pa);
void drawIAQ(float iaq);
void drawIAQAccuracy(uint8_t accuracy);
void drawBatteryVoltage(float batteryVoltage);
void drawStatusBar(float batteryVoltage, boolean sensorOk, BLEStatus bleStatus);
BatteryLevel getBatteryLevel(float batteryVoltage);
IAQState getIaqState(float iaq);