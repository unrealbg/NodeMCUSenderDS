#include <Arduino.h>

const int Dry = 1024;
const int Wet = 1005;
const int SensorPin = A0;
int soilMoistureValue = 0;
float soilmoisturepercent = 0;
float Temperature;
float Humidity;
float lux;
const long utcOffsetInSeconds = 10800;
void sendTempHumidity();
void sendLight();
void sendSoilMoisture();
void setup_mqtt();
void sendUptime();