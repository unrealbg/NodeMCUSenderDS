/*
 Name:    NodeMCU_DS + MQTT + Sensors
 Created: 16-Mar-22 10:00:44
 Author:  Zhelyazkov
*/

#include <Arduino.h>
#include <mqtt-auth.h>
#include <user-variables.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>  
#include <PubSubClient.h>
#include <DHTesp.h>
#include <BH1750.h>
#include <SPI.h>
#include <Wire.h>
#include <NTPClient.h>

WiFiUDP ntpUDP;
WiFiClient espClient;
PubSubClient client(espClient);
BH1750 lightMeter(0x23);
DHTesp dht;
AsyncWebServer server(80);
DNSServer dns;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void sendTempHumidity()
{
    Humidity = dht.getHumidity();
    Temperature = dht.getTemperature();

    if (isnan(Humidity) || isnan(Temperature)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    client.publish("home/wemos/temp", String(Temperature).c_str(), true);
    client.publish("home/wemos/humidity", String(Humidity).c_str(), true);

    delay(1000);
}

void sendLight() {
    lux = lightMeter.readLightLevel();

    client.publish("home/wemos/light", String(lux).c_str(), true);

    delay(1000);
}

void sendSoilMoisture() {

    soilMoistureValue = analogRead(SensorPin);
    soilmoisturepercent = map(soilMoistureValue, Dry, Wet, 0, 100);

    if (soilmoisturepercent > 100)
    {
        client.publish("home/wemos/soil", "100", true);
    }
    else if (soilmoisturepercent < 0)
    {
        client.publish("home/wemos/soil", "0", true);
    }
    else if (soilmoisturepercent >= 0 && soilmoisturepercent <= 100)
    {
        client.publish("home/wemos/soil", String(soilmoisturepercent).c_str(), true);
    }
}

void setup_mqtt() {
    client.setServer(mqttServer, mqttPort);

    while (!client.connected()) {
        Serial.println("Connecting to MQTT...");

        if (client.connect("NodeMCUDeepSleep", mqttUser, mqttPassword)) {

            Serial.println("connected");

        }
        else {

            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
}

bool sendUp(void*) {
    sendUptime();
    return true;
}

void sendUptime() {
    timeClient.update();
    //client.publish("home/wemos/dateNow", String(timeClient.getDay()).c_str(), true);
    int dayOfWeekNum = timeClient.getDay();
    switch (dayOfWeekNum)
    {
        case 0: client.publish("home/wemos/dayOfWeek", "Sunday", true); break;
        case 1: client.publish("home/wemos/dayOfWeek", "Monday", true); break;
        case 2: client.publish("home/wemos/dayOfWeek", "Tuesday", true); break;
        case 3: client.publish("home/wemos/dayOfWeek", "Wednesday", true); break;
        case 4: client.publish("home/wemos/dayOfWeek", "Thursday", true); break;
        case 5: client.publish("home/wemos/dayOfWeek", "Friday", true); break;
        case 6: client.publish("home/wemos/dayOfWeek", "Saturday", true); break;
        default:
                break;
    }

    client.publish("home/wemos/timeNow", String(timeClient.getFormattedTime()).c_str(), true);
}

void setup()
{
    // Debug console
    Serial.begin(115200);
    //Serial.begin(9600);
    Serial.setTimeout(2000);

    Wire.begin(D2, D1);
    AsyncWiFiManager wifiManager(&server, &dns);
    wifiManager.autoConnect();
    setup_mqtt();
    timeClient.begin();
    dht.setup(0, DHTesp::DHT22);
    lightMeter.begin();
    client.subscribe("#");
    sendTempHumidity();
    sendUptime();
    sendLight();
    sendSoilMoisture();
    Serial.println("deep sleep for 30 min");
    while (isnan(Temperature) && lux == -2)
    {
        sendTempHumidity();
        sendLight();
    }
    ESP.deepSleep(1800e6);
}

void loop()
{
}