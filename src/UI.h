#include <Arduino.h>

enum ScreenArea { TOP, BOTTOM };
enum BatteryLevel { BAT_HIGH, BAT_MEDIUM, BAT_LOW };

void initUI();
void invertColorScheme();
void drawTemperature(float temp);
void drawAltitude(int16_t altitude);
void drawHumidity(uint8_t hum);
void drawPressure(int32_t pa);
void drawTitleSprite(const char *text, ScreenArea area);
void drawMeasurementSprite(const char *value, const char *unitOfMeasure, ScreenArea area);
void drawMeasurementIcon(const char *value, ScreenArea area);
void drawBatteryStatus(const char *value);
void drawBatteryStatus(const char *value, uint16_t textColor);
void drawBTStatus(boolean on);
void drawStatusBar(BatteryLevel battery, boolean btConnected);