#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define I2C_ADDR 0x27
#define BACKLIGHT_PIN 3

const char* ssid = "Project";
const char* password = "12345678";
const char* server = "api.thingspeak.com";
const String apiKey = "OEL5UOKAN250I5UT";

const int rainSensorPin = 35;
const int ldrSensorPin = 34;
#define DHT11PIN 18
#define DHTTYPE DHT11

Adafruit_BMP085 bmp;
DHT dht(DHT11PIN, DHTTYPE);

int buttonPin = 2;
int buttonState = 0;
bool bmpConnected = true;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085/BMP180 sensor, check wiring!");
    bmpConnected = false;
  }

  dht.begin();
}

void loop() {
  buttonState = digitalRead(buttonPin);

  if (buttonState == LOW) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    float temperature = bmpConnected ? bmp.readTemperature() : 0.0;
    float pressure = bmpConnected ? bmp.readPressure() : 0.0;
    float altitude = bmpConnected ? bmp.readAltitude() : 0.0;
    float sealevelPressure = bmpConnected ? bmp.readSealevelPressure() : 0.0;
    float humidity = dht.readHumidity();
    float outsideTemp = dht.readTemperature();
    int rainSensorValue = 100 - analogRead(rainSensorPin);
    int ldrSensorValue = analogRead(ldrSensorPin);

    float rainPercent = map(rainSensorValue, 0, 4095, 0, 100);
    float ldrPercent = map(ldrSensorValue, 0, 4095, 0, 100);

    // Display data on Serial Monitor
    Serial.println("Data:");
    if (bmpConnected) {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" *C");
      Serial.print("Pressure: ");
      Serial.print(pressure);
      Serial.println(" Pa");
      Serial.print("Altitude: ");
      Serial.print(altitude);
      Serial.println(" meters");
      Serial.print("Pressure at sealevel (calculated): ");
      Serial.print(sealevelPressure);
      Serial.println(" Pa");
    } else {
      Serial.println("BMP sensor not connected.");
    }
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    Serial.print("Outside Temperature: ");
    Serial.print(outsideTemp);
    Serial.println(" *C");
    Serial.print("Rain Sensor: ");
    Serial.print(rainPercent);
    Serial.println(" %");
    Serial.print("LDR Sensor: ");
    Serial.print(ldrPercent);
    Serial.println(" %");
    Serial.println();

    sendDataToThingSpeak(temperature, pressure, altitude, sealevelPressure, outsideTemp, humidity, rainPercent, ldrPercent);

    delay(10000);
  }
}

void sendDataToThingSpeak(float temperature, float pressure, float altitude, float sealevelPressure, float outsideTemp, float humidity, float rainPercent, float ldrPercent) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "http://api.thingspeak.com/update?api_key=" + apiKey;
    url += "&field1=" + String(temperature);
    url += "&field2=" + String(pressure);
    url += "&field3=" + String(altitude);
    url += "&field4=" + String(sealevelPressure);
    url += "&field5=" + String(outsideTemp);
    url += "&field6=" + String(humidity);
    url += "&field7=" + String(rainPercent);
    url += "&field8=" + String(ldrPercent);

    Serial.println("Sending data to ThingSpeak...");
    Serial.println(url);

    http.begin(url);

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      Serial.println("Data sent to ThingSpeak successfully!");
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected! Unable to send data to ThingSpeak.");
  }
}
