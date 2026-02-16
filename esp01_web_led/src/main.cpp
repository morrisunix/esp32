#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "index_html.h"

// WiFi Credentials
const char* ssid = "roku";
const char* password = "Linux.456";

ESP8266WebServer server(80);
const int ledPin = 2; // GPIO2

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleToggle() {
  if (server.hasArg("state")) {
    int state = server.arg("state").toInt();
    digitalWrite(ledPin, state ? LOW : HIGH); 
    StaticJsonDocument<100> doc;
    doc["state"] = state;
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }
}

void handleStatus() {
  StaticJsonDocument<100> doc;
  doc["state"] = (digitalRead(ledPin) == LOW) ? 1 : 0;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void setup() {
  Serial.begin(115200);
  
  // CRITICAL: Give power time to stabilize before WiFi kicks in
  for(int i = 0; i < 10; i++) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n\n[BOOT] Power Stabilized.");

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); 

  WiFi.persistent(false); // Don't write to flash every time
  WiFi.mode(WIFI_STA);
  
  // REDUCE POWER: Try to prevent FTDI brownout
  WiFi.setOutputPower(10); // Lower power (0-20.5 range)
  
  Serial.printf("[WIFI] Connecting to: %s\n", ssid);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(1000); // Slower polling to save power
    Serial.print(WiFi.status()); // Print status code (7=connecting, 3=connected)
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] Connected!");
    Serial.print("[WIFI] IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WIFI] Failed to connect.");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_POST, handleToggle);
  server.on("/status", HTTP_GET, handleStatus);
  server.begin();
  
  Serial.println("[HTTP] Server ready.");
}

void loop() {
  server.handleClient();
  
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 10000) {
    lastUpdate = millis();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("[OK] RSSI: %d | IP: %s\n", WiFi.RSSI(), WiFi.localIP().toString().c_str());
    }
  }
}