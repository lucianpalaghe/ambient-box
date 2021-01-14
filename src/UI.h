#include <Arduino.h>

enum ScreenArea { TOP, BOTTOM };
enum BatteryLevel { BAT_LOW, BAT_MEDIUM, BAT_HIGH, BAT_CHARGING, BAT_UNKNOWN };
enum IAQAccuracy { IAQACC_STABILIZING, IAQACC_LOW, IAQACC_MEDIUM, IAQACC_HIGH };

void initUI();
void invertColorScheme();
void drawTemperature(float temp);
void drawAltitude(int16_t altitude);
void drawHumidity(float hum);
void drawPressure(float pa);
void drawIAQ(float iaq);
void drawIAQAccuracy(uint8_t accuracy);
void drawStatusBar(BatteryLevel battery, boolean sensorOk, boolean btConnected);