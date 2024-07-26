//ESP32 code for weather station 
// created by Amit Kumar Singh

#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define DHT11PIN 18
#define DHTTYPE DHT11

const char* ssid = "Project";
const char* password = "12345678";
const char* server = "api.thingspeak.com";
const String apiKey = "OEL5UOKAN250I5UT";

const int sensorPin1 = 35; // Rain sensor pin
#define DHT11PIN 18
#define DHTTYPE DHT11

Adafruit_BMP085 bmp;
DHT dht(DHT11PIN, DHTTYPE);

int buttonPin = 2; 
int buttonState = 0; 
int displayMode = 1; 

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP); 

  Serial.println("Initializing...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085/BMP180 sensor, check wiring!");
    while (1) {}
  }

  dht.begin();
}

void loop() {
  buttonState = digitalRead(buttonPin); 

  if (buttonState == LOW) { 
    changeDisplayMode();    
    delay(500);             
  }

  if (WiFi.status() == WL_CONNECTED) {
    float temperatureBMP = bmp.readTemperature();
    float pressure = bmp.readPressure();
    float temperatureDHT = dht.readTemperature();
    float humidity = dht.readHumidity();
    int rainSensorValue = analogRead(sensorPin1);

    // Display data on Serial Monitor
    Serial.println("Data:");
    Serial.print("Temperature (BMP180): ");
    Serial.print(temperatureBMP);
    Serial.println(" *C");
    Serial.print("Temperature (DHT11): ");
    Serial.print(temperatureDHT);
    Serial.println(" *C");
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" Pa");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    Serial.print("Rain Sensor: ");
    Serial.print(rainSensorValue);
    Serial.println();
    
    sendDataToThingSpeak(temperatureBMP, temperatureDHT, pressure, humidity, rainSensorValue);

    delay(10000); 
  }
}

void sendDataToThingSpeak(float temperatureBMP, float temperatureDHT, float pressure, float humidity, int rainSensorValue) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "http://api.thingspeak.com/update?api_key=" + apiKey;
    url += "&field1=" + String(temperatureBMP);
    url += "&field2=" + String(temperatureDHT);
    url += "&field3=" + String(pressure);
    url += "&field4=" + String(humidity);
    url += "&field5=" + String(rainSensorValue);

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

void changeDisplayMode() {
  displayMode++; 

  if (displayMode > 5) { 
    displayMode = 1;
  }
}
