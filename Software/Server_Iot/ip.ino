//ESP32 code for ip
// created by Amit Kumar Singh
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "Project";
const char* password = "12345678";

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Wait until the ESP32 is connected to the Wi-Fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Display the ESP32 IP address
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
}
