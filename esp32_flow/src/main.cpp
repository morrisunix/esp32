#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <esp_task_wdt.h>
#include "index_html.h"

// Configuration
const char* ssid = "roku";
const char* password = "Linux.456";
#define FLOW_SENSOR_PIN 34 
#define RELAY_PIN 13
#define VALVE_PIN 16
#define WDT_TIMEOUT 5 

// Global Objects
WebServer server(80);
WebSocketsServer webSocket(81);
Preferences preferences;

// Volatile System State
struct SystemState {
    float accumulatedVolume = 0;
    float currentFlow = 0;
    float volumeTarget = 1000.0;
    bool relayActive = false;
    bool valveActive = false;
    bool targetReached = false;
    unsigned long relayStartTime = 0;
    unsigned long accumulatedTimeMs = 0;
} state;

// Kalman Filter for Noise reduction
class KalmanFilter {
    float _q, _r, _p, _x, _k;
public:
    KalmanFilter(float q, float r, float p, float initial) : _q(q), _r(r), _p(p), _x(initial) {}
    float update(float measurement) {
        _p = _p + _q;
        _k = _p / (_p + _r);
        _x = _x + _k * (measurement - _x);
        _p = (1 - _k) * _p;
        return _x;
    }
} flowFilter(0.01, 0.1, 1.0, 0.0);

// Function Prototypes
void broadcastStatus();
void saveVolumeToNVS();

// --- CORE 1: High Priority Flow Integration Task ---
void flowControlTask(void * pvParameters) {
    unsigned long lastCalc = millis();
    esp_task_wdt_add(NULL); // Add current task to WDT

    for(;;) {
        esp_task_wdt_reset(); // Feed WDT
        
        unsigned long now = millis();
        int raw = analogRead(FLOW_SENSOR_PIN);
        
        // Industrial Mapping (4096 is 12-bit ADC)
        float raw_flow = (float)(raw - 744) * 100.0 / (3720 - 744);
        if (raw_flow < 0) raw_flow = 0;
        state.currentFlow = flowFilter.update(raw_flow);

        if (state.relayActive && !state.targetReached) {
            float timeStep = (now - lastCalc) / 1000.0;
            state.accumulatedVolume += (state.currentFlow / 60.0) * timeStep;

            if (state.accumulatedVolume >= state.volumeTarget) {
                state.accumulatedVolume = state.volumeTarget;
                state.targetReached = true;
                state.relayActive = false;
                digitalWrite(RELAY_PIN, LOW);
                Serial.println("[CRITICAL] Target Reached. Pump OFF.");
                broadcastStatus();
            }
        }
        lastCalc = now;
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz sampling
    }
}

// --- CORE 0: Network & UI Management ---
void handleRoot() { server.send_P(200, "text/html", INDEX_HTML); }

void broadcastStatus() {
    // Consolidated Status Update
    StaticJsonDocument<512> doc;
    
    unsigned long currentSession = state.accumulatedTimeMs;
    if (state.relayActive) currentSession += (millis() - state.relayStartTime);

    doc["type"] = "status";
    doc["flow"] = state.currentFlow;
    doc["vol"] = state.accumulatedVolume;
    doc["target"] = state.volumeTarget;
    doc["elapsed"] = currentSession / 1000;
    doc["targetReached"] = state.targetReached;
    doc["uptime"] = millis() / 1000;
    doc["relay"] = state.relayActive;
    doc["valve"] = state.valveActive;

    String msg;
    serializeJson(doc, msg);
    webSocket.broadcastTXT(msg);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_TEXT) {
        String text = String((char*)payload);
        if (text.startsWith("toggle:")) {
            int pin = text.substring(7).toInt();
            if (pin == RELAY_PIN) {
                state.relayActive = !state.relayActive;
                digitalWrite(RELAY_PIN, state.relayActive ? HIGH : LOW);
                if (state.relayActive) {
                    if (state.targetReached) { 
                        state.accumulatedVolume = 0; 
                        state.accumulatedTimeMs = 0; 
                        state.targetReached = false; 
                    }
                    state.relayStartTime = millis();
                } else {
                    state.accumulatedTimeMs += (millis() - state.relayStartTime);
                    saveVolumeToNVS();
                }
            } else if (pin == VALVE_PIN) {
                state.valveActive = !state.valveActive;
                digitalWrite(VALVE_PIN, state.valveActive ? HIGH : LOW);
            }
            
            // Sync UI
            String sync = "{\"type\":\"pinState\",\"pin\":" + String(pin) + ",\"state\":" + String(digitalRead(pin)) + "}";
            webSocket.broadcastTXT(sync);
            broadcastStatus();
        } else if (text.startsWith("setTarget:")) {
            // Safety Rule 1: Don't allow changes while running
            if (state.relayActive) {
                Serial.println("[SAFETY] Target change REJECTED: System is running.");
                return;
            }

            float newTarget = text.substring(10).toFloat();
            
            // Safety Rule 2: New target must be greater than current volume
            if (newTarget > state.accumulatedVolume) {
                state.volumeTarget = newTarget;
                state.targetReached = false;
                broadcastStatus();
                Serial.printf("[SYSTEM] Target updated to %.1f L\n", state.volumeTarget);
            } else {
                Serial.printf("[SAFETY] Target change REJECTED: New target (%.1f) <= Current (%.1f)\n", newTarget, state.accumulatedVolume);
                // Force sync frontend to revert value
                broadcastStatus();
            }
        } else if (text == "resetBatch") {
            if (state.relayActive) {
                Serial.println("[SAFETY] Reset REJECTED: System is running.");
                return;
            }
            state.accumulatedVolume = 0;
            state.accumulatedTimeMs = 0;
            state.targetReached = false;
            saveVolumeToNVS();
            broadcastStatus();
        }
    }
}

void saveVolumeToNVS() {
    preferences.putFloat("lastVol", state.accumulatedVolume);
    Serial.println("[STORAGE] Volume backed up to NVS");
}

void setup() {
    Serial.begin(115200);
    delay(500);

    // Watchdog Setup
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL); // Main loop watchdog

    // Load persisted data
    preferences.begin("flow", false);
    state.accumulatedVolume = preferences.getFloat("lastVol", 0);
    Serial.printf("[BOOT] Resumed Volume: %.2f L\n", state.accumulatedVolume);

    pinMode(RELAY_PIN, OUTPUT);
    pinMode(VALVE_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(VALVE_PIN, LOW);

    // ADC Resolution for Industrial accuracy
    analogReadResolution(12);

    // WiFi Robustness & Performance
    WiFi.setHostname("esp32-industrial-flow");
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false); // CRITICAL: Performance fix for laggy web servers
    WiFi.begin(ssid, password);

    server.on("/", handleRoot);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    // Task Creation
    xTaskCreatePinnedToCore(flowControlTask, "FlowTask", 4096, NULL, 5, NULL, 1);
    
    Serial.println("[SYSTEM] Multi-core Industrial Controller ready.");
}

void loop() {
    esp_task_wdt_reset(); // Primary Loop Watchdog reset
    server.handleClient();
    webSocket.loop();

    // Periodic UI refreshes (Core 0 handles the radio, but loop runs on Core 1)
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 250) { // Increased from 2000ms to 250ms for smooth updates
        lastUpdate = millis();
        broadcastStatus();
        
        // Save to NVS every 30 seconds to prevent flash wear but allow recovery
        static unsigned long lastSave = 0;
        if (millis() - lastSave > 30000) {
            lastSave = millis();
            saveVolumeToNVS();
        }
        
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[WARN] Connection lost. Auto-reconnecting...");
        }
    }
}
