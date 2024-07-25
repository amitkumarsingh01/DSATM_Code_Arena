#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <ESPAsyncWebServer.h>

#define DHT11PIN 18
#define DHTTYPE DHT11
#define LIGHT_SENSOR_PIN 33
#define SERVO_PIN 13
#define PIR_SENSOR_PIN 27
#define RELAY_PIN 26

const char* ssid = "Project";
const char* password = "12345678";
const char* server = "api.thingspeak.com";
const String apiKey = "OEL5UOKAN250I5UT";

const int rainSensorPin = 32; // Rain sensor pin

Adafruit_BMP085 bmp;
DHT dht(DHT11PIN, DHTTYPE);
Servo myServo;

AsyncWebServer server(80);

int buttonPin = 2; 
int buttonState = 0; 
int displayMode = 1;

bool automatedMode = true; // Automated mode by default
bool lightStatus = false; // Light off by default
int servoPosition = 0; // Servo at 0 degrees by default

void setup() {
  Serial.begin(115200);
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

  server.on("/toggle_automated", HTTP_GET, [](AsyncWebServerRequest *request){
    automatedMode = !automatedMode;
    request->send(200, "text/plain", automatedMode ? "Automated mode" : "Manual mode");
  });

  server.on("/toggle_light", HTTP_GET, [](AsyncWebServerRequest *request){
    lightStatus = !lightStatus;
    digitalWrite(RELAY_PIN, lightStatus ? HIGH : LOW);
    request->send(200, "text/plain", lightStatus ? "Light ON" : "Light OFF");
  });

  server.on("/toggle_dim", HTTP_GET, [](AsyncWebServerRequest *request){
    servoPosition = (servoPosition == 90) ? 180 : 90;
    myServo.write(servoPosition);
    request->send(200, "text/plain", servoPosition == 90 ? "Dim" : "Bright");
  });

  server.on("/get_data", HTTP_GET, [](AsyncWebServerRequest *request){
    String jsonData = getData();
    request->send(200, "application/json", jsonData);
  });

  server.begin();
}

void loop() {
  buttonState = digitalRead(buttonPin); 

  if (buttonState == LOW) { 
    changeDisplayMode();    
    delay(500);             
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (automatedMode) {
      // Read Weather Data
      float temperatureBMP = bmp.readTemperature();
      float pressure = bmp.readPressure();
      float temperatureDHT = dht.readTemperature();
      float humidity = dht.readHumidity();
      int rainSensorValue = analogRead(rainSensorPin);
      int analogValue = analogRead(LIGHT_SENSOR_PIN);

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
      Serial.print("Analog Value (LDR): ");
      Serial.println(analogValue);
      
      // Send Weather Data to ThingSpeak
      sendDataToThingSpeak(temperatureBMP, temperatureDHT, pressure, humidity, rainSensorValue, analogValue);

      // Read PIR Data
      int pirValue = digitalRead(PIR_SENSOR_PIN);

      Serial.print("PIR Value = ");
      Serial.print(pirValue);

      if (analogValue > 2045) {
        Serial.println(" => Bright");
        digitalWrite(RELAY_PIN, HIGH); // Turn relay off
      } else {
        Serial.println(" => Dark");
        digitalWrite(RELAY_PIN, LOW); // Turn relay on

        if (pirValue == HIGH) {
          myServo.write(180); // Move servo to 180 degrees
        } else {
          myServo.write(0); // Move servo to 0 degrees
        }
      }

      delay(500); // Delay between data readings
    }
  }
}

void sendDataToThingSpeak(float temperatureBMP, float temperatureDHT, float pressure, float humidity, int rainSensorValue, int analogValue) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "http://api.thingspeak.com/update?api_key=" + apiKey;
    url += "&field1=" + String(temperatureBMP);
    url += "&field2=" + String(temperatureDHT);
    url += "&field3=" + String(pressure);
    url += "&field4=" + String(humidity);
    url += "&field5=" + String(rainSensorValue);
    url += "&field6=" + String(analogValue);

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

String getData() {
  float temperatureBMP = bmp.readTemperature();
  float pressure = bmp.readPressure();
  float temperatureDHT = dht.readTemperature();
  float humidity = dht.readHumidity();
  int rainSensorValue = analogRead(rainSensorPin);
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  int pirValue = digitalRead(PIR_SENSOR_PIN);

  String jsonData = "{";
  jsonData += "\"temperatureBMP\":" + String(temperatureBMP) + ",";
  jsonData += "\"pressure\":" + String(pressure) + ",";
  jsonData += "\"temperatureDHT\":" + String(temperatureDHT) + ",";
  jsonData += "\"humidity\":" + String(humidity) + ",";
  jsonData += "\"rainSensor\":" + String(rainSensorValue) + ",";
  jsonData += "\"lightSensor\":" + String(analogValue) + ",";
  jsonData += "\"pirSensor\":" + String(pirValue) + ",";
  jsonData += "\"relayStatus\":" + String(digitalRead(RELAY_PIN)) + ",";
  jsonData += "\"servoPosition\":" + String(servoPosition) + ",";
  jsonData += "\"lightStatus\":" + String(lightStatus);
  jsonData += "}";

  return jsonData;
}
