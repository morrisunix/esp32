#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <DHT.h>
#include "index_html.h"

// WiFi credentials
const char* ssid = "roku";
const char* password = "Linux.456";

// DHT Sensor Setup
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Ultrasonic Sensor Setup
#define TRIG_PIN 12
#define ECHO_PIN 14

// Kalman Filter Class for Flowmeter
class KalmanFilter {
  public:
    KalmanFilter(float q, float r, float p, float initial_value) {
      _q = q; _r = r; _p = p; _x = initial_value;
    }
    float update(float measurement) {
      _p = _p + _q;
      _k = _p / (_p + _r);
      _x = _x + _k * (measurement - _x);
      _p = (1 - _k) * _p;
      return _x;
    }
  private:
    float _q, _r, _p, _x, _k;
};

KalmanFilter flowKalman(0.01, 0.1, 1.0, 0.0);

// GPIO Setup (Output pins)
const int pins[] = {13, 16};
const int numPins = 2;
bool pinStates[17] = {false};

// Volume and Time tracking
float accumulatedVolume = 0;
unsigned long relayStartTime = 0;
unsigned long lastVolumeCalcTime = 0;
unsigned long accumulatedTimeMs = 0;
bool volumeTargetReached = false;
float volumeTarget = 1000.0; // Default: 1000 Liters

unsigned long lastDHTTime = 0;
unsigned long lastUltraTime = 0;
unsigned long lastFlowTime = 0;

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

void handleRoot() {
  Serial.printf("HTTP GET request to / from client: %s\n", server.client().remoteIP().toString().c_str());
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleNotFound() {
  Serial.printf("HTTP %s request to %s from client: %s (Status: 404)\n", 
                (server.method() == HTTP_GET) ? "GET" : "POST", 
                server.uri().c_str(), 
                server.client().remoteIP().toString().c_str());
  server.send(404, "text/plain", "Not Found");
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0 || duration > 30000) return -1;
  if (duration < 60) return 0.0; 
  return duration * 0.034 / 2;
}

float getFlow() {
  int raw = analogRead(A0);
  float raw_flow = (float)(raw - 186) * 100.0 / (930 - 186);
  if (raw_flow < 0) raw_flow = 0;
  return flowKalman.update(raw_flow);
}

void broadcastVolumeUpdate() {
  unsigned long currentSessionTime = accumulatedTimeMs;
  if (pinStates[13]) {
    currentSessionTime += (millis() - relayStartTime);
  }
  unsigned long elapsedSec = currentSessionTime / 1000;
  
  String msg = "{\"type\":\"volumeUpdate\", \"vol\": " + String(accumulatedVolume, 2) + 
               ", \"target\": " + String(volumeTarget, 2) + 
               ", \"elapsed\": " + String(elapsedSec) + 
               ", \"targetReached\": " + String(volumeTargetReached ? "true" : "false") + "}";
  webSocket.broadcastTXT(msg);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %s\n", num, ip.toString().c_str());
      for(int i=0; i<numPins; i++) {
        String msg = "{\"type\":\"pinState\", \"pin\": " + String(pins[i]) + ", \"state\": " + String(pinStates[pins[i]] ? "true" : "false") + "}";
        webSocket.sendTXT(num, msg);
      }
      broadcastVolumeUpdate();
      break;
    }
    case WStype_TEXT: {
      String text = String((char*)payload);
      if (text.startsWith("toggle:")) {
        int pin = text.substring(7).toInt();
        bool allowed = false;
        for(int i=0; i<numPins; i++) {
          if(pins[i] == pin) { allowed = true; break; }
        }
        if (allowed) {
          pinStates[pin] = !pinStates[pin];
          digitalWrite(pin, pinStates[pin] ? HIGH : LOW);
          
          if (pin == 13) {
             if (pinStates[13]) {
                // RESUME or START
                if (volumeTargetReached) {
                    accumulatedVolume = 0;
                    accumulatedTimeMs = 0;
                    volumeTargetReached = false;
                }
                relayStartTime = millis();
                lastVolumeCalcTime = millis();
                Serial.println("Relay ON: Batch operation Running");
             } else {
                // PAUSE
                accumulatedTimeMs += (millis() - relayStartTime);
                Serial.printf("Relay OFF: Paused at %.2f L\n", accumulatedVolume);
             }
          }
          
          String msg = "{\"type\":\"pinState\", \"pin\": " + String(pin) + ", \"state\": " + String(pinStates[pin] ? "true" : "false") + "}";
          webSocket.broadcastTXT(msg);
          broadcastVolumeUpdate();
        }
      } else if (text.startsWith("setTarget:")) {
        float newTarget = text.substring(10).toFloat();
        if (newTarget >= 1.0 && newTarget <= 1000000.0) {
          volumeTarget = newTarget;
          volumeTargetReached = false;
          broadcastVolumeUpdate();
        }
      } else if (text.equals("resetBatch")) {
        accumulatedVolume = 0;
        accumulatedTimeMs = 0;
        volumeTargetReached = false;
        if (pinStates[13]) {
           relayStartTime = millis();
           lastVolumeCalcTime = millis();
        }
        Serial.println("Batch Reset requested");
        broadcastVolumeUpdate();
      }
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  for(int i=0; i<numPins; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("Servers started (HTTP: 80, WS: 81)");
}

void loop() {
  server.handleClient();
  webSocket.loop();

  // Volume Integration (Calculated every 100ms for precision)
  if (pinStates[13] && !volumeTargetReached) {
    if (millis() - lastVolumeCalcTime >= 100) {
      float currentFlow = getFlow(); // L/min
      float timeStepSeconds = (millis() - lastVolumeCalcTime) / 1000.0;
      float volumeStep = (currentFlow / 60.0) * timeStepSeconds; // Convert L/min to L/sec * sec
      accumulatedVolume += volumeStep;
      lastVolumeCalcTime = millis();

      // Check for target completion
      if (accumulatedVolume >= volumeTarget) {
        accumulatedVolume = volumeTarget; // Cap at target for display
        accumulatedTimeMs += (millis() - relayStartTime); // Final time update
        volumeTargetReached = true;
        pinStates[13] = false;
        digitalWrite(13, LOW);
        Serial.println("TARGET REACHED: Powering off Relay 13");
        
        // Broadcast final state
        String msg = "{\"type\":\"pinState\", \"pin\": 13, \"state\": false}";
        webSocket.broadcastTXT(msg);
        broadcastVolumeUpdate();
      }
      // Broadcast volume update every 1 second to web
      static unsigned long lastVolBroadcast = 0;
      if (millis() - lastVolBroadcast > 1000) {
        lastVolBroadcast = millis();
        broadcastVolumeUpdate();
      }
    }
  }

  if (millis() - lastDHTTime > 2000) {
    lastDHTTime = millis();
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      String msg = "{\"type\":\"sensor\", \"temp\": " + String(t) + ", \"hum\": " + String(h) + "}";
      webSocket.broadcastTXT(msg);
      Serial.printf("Sensor: Temp: %.1f °C | Humidity: %.1f %%\n", t, h);
    }
  }

  if (millis() - lastUltraTime > 500) {
    lastUltraTime = millis();
    float dist = getDistance();
    if (dist > 0.5) {
      String msg = "{\"type\":\"distance\", \"val\": " + String(dist) + "}";
      webSocket.broadcastTXT(msg);
      Serial.printf("Distance: %.1f cm\n", dist);
    }
  }

  if (millis() - lastFlowTime > 1000) {
    lastFlowTime = millis();
    float flow = getFlow();
    String msg = "{\"type\":\"flow\", \"val\": " + String(flow) + "}";
    webSocket.broadcastTXT(msg);
    Serial.printf("Flow: %.1f L/min | Volume: %.2f L\n", flow, accumulatedVolume);
  }
}
