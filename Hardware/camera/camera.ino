#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid = "Project";
const char* password = "12345678";

WebServer server(80); 

void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><style>body{margin:0;}</style></head>";
  html += "<body>";
  html += "<img id='stream' src='/stream' style='width:100vw; height:56.25vw; object-fit:cover;' />";
  html += "<button onclick='saveImage()' style='position:absolute;top:10px;left:10px;'>Save Image</button>";
  html += "<script>";
  html += "function saveImage() {";
  html += "var link = document.createElement('a');";
  html += "link.href = document.getElementById('stream').src;";
  html += "link.download = 'snapshot.jpg';";
  html += "link.click();";
  html += "}";
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleStream() {
  WiFiClient client = server.client();
  String boundary = "frame";
  
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=" + boundary + "\r\n";
  response += "\r\n";
  client.write(response.c_str());

  while (true) {
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    response = "--" + boundary + "\r\n";
    response += "Content-Type: image/jpeg\r\n";
    response += "Content-Length: " + String(fb->len) + "\r\n";
    response += "\r\n";
    client.write(response.c_str());
    client.write(fb->buf, fb->len);
    client.write("\r\n");

    esp_camera_fb_return(fb);

    if (!client.connected()) {
      break;
    }

    delay(100);  // Adjust the delay to control the frame rate
  }
}

void setup() {
  Serial.begin(9600);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize the camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Adjust the frame size and JPEG quality for better performance
  config.frame_size = FRAMESIZE_HD; // 1280x720, which is 16:9
  config.jpeg_quality = 10;         // Lower quality for reduced lag
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Start the web server
  server.on("/", handleRoot);
  server.on("/stream", handleStream);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
