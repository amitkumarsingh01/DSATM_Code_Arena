//ESP32 code for light
// created by Amit Kumar Singh

#include <ESP32Servo.h>

#define LIGHT_SENSOR_PIN 33
#define SERVO_PIN 13
#define PIR_SENSOR_PIN 27
#define RELAY_PIN 26

Servo myServo;

void setup() {
  Serial.begin(115200);

  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  myServo.attach(SERVO_PIN);
  digitalWrite(RELAY_PIN, LOW); // Ensure relay is initially off
}

void loop() {
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  int pirValue = digitalRead(PIR_SENSOR_PIN);

  Serial.print("Analog Value = ");
  Serial.print(analogValue);
  Serial.print(" | PIR Value = ");
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

  delay(500);
}