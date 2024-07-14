#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>

#define DHT11PIN 18
#define DHTTYPE DHT11
#define LIGHT_SENSOR_PIN 34
#define SERVO_PIN 13
#define PIR_SENSOR_PIN 27
#define RELAY_PIN 26

const char* ssid = "Project";
const char* password = "12345678";
const char* server = "api.thingspeak.com";
const String apiKey = "OEL5UOKAN250I5UT";

const int rainSensorPin = 35; // Rain sensor pin

Adafruit_BMP085 bmp;
DHT dht(DHT11PIN, DHTTYPE);
Servo myServo;

int buttonPin = 2; 
int buttonState = 0; 
int displayMode = 1;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  myServo.attach(SERVO_PIN);
  digitalWrite(RELAY_PIN, LOW); // Ensure relay is initially off
  
  Serial.println("Initializing...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
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
    // Read Weather Data
    float temperatureBMP = bmp.readTemperature();
    float pressure = bmp.readPressure();
    float temperatureDHT = dht.readTemperature();
    float humidity = dht.readHumidity();
    int rainSensorValue = analogRead(rainSensorPin);

    // Display Weather Data on Serial Monitor
    Serial.println("Weather Data:");
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
    Serial.println(rainSensorValue);
    
    // Send Weather Data to ThingSpeak
    sendDataToThingSpeak(temperatureBMP, temperatureDHT, pressure, humidity, rainSensorValue);

    // Read Light and PIR Data
    int analogValue = analogRead(LIGHT_SENSOR_PIN);
    int pirValue = digitalRead(PIR_SENSOR_PIN);

    Serial.print("Analog Value = ");
    Serial.print(analogValue);
    Serial.print(" | PIR Value = ");
    Serial.print(pirValue);

    if (analogValue < 2045) {
      Serial.println(" => Dark");
      digitalWrite(RELAY_PIN, LOW); // Turn relay off
      myServo.write(0); // Move servo to 0 degrees
    } else {
      Serial.println(" => Very bright");
      digitalWrite(RELAY_PIN, HIGH); // Turn relay on

      if (pirValue == HIGH) {
        myServo.write(180); // Move servo to 180 degrees
      } else {
        myServo.write(0); // Move servo to 0 degrees
      }
    }

    delay(10000); // Delay between data readings
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
