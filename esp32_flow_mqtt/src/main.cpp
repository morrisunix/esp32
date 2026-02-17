#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <esp_task_wdt.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "index_html.h"
#include "secrets.h"

#define FIRMWARE_VERSION "2.1.0"

// Hardware Configuration
#define FLOW_SENSOR_PIN 34 
#define RELAY_PIN 13
#define VALVE_PIN 16
#define WDT_TIMEOUT 60 

// Network Globals
const char* ssid = "roku";
const char* password = "Linux.456";

WiFiClient net;
PubSubClient client(net);

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

void broadcastStatus();
void saveVolumeToNVS();
void connectToMQTT();
void mqttTask(void * pvParameters);
void syncTime();

// --- CORE 1: High Priority Flow Integration Task ---
void flowControlTask(void * pvParameters) {
    unsigned long lastCalc = millis();
    esp_task_wdt_add(NULL); 
    Serial.println("[TASK] FlowControlTask started on Core 1");

    for(;;) {
        esp_task_wdt_reset(); 
        unsigned long now = millis();
        int raw = analogRead(FLOW_SENSOR_PIN);
        
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
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void handleRoot() { server.send_P(200, "text/html", INDEX_HTML); }

void broadcastStatus() {
    StaticJsonDocument<256> volDoc;
    unsigned long currentSession = state.accumulatedTimeMs;
    if (state.relayActive) currentSession += (millis() - state.relayStartTime);

    volDoc["type"] = "volumeUpdate";
    volDoc["vol"] = state.accumulatedVolume;
    volDoc["target"] = state.volumeTarget;
    volDoc["elapsed"] = currentSession / 1000;
    volDoc["targetReached"] = state.targetReached;
    volDoc["uptime"] = millis() / 1000;
    volDoc["relayActive"] = state.relayActive;
    volDoc["valveActive"] = state.valveActive;

    String volMsg;
    serializeJson(volDoc, volMsg);
    webSocket.broadcastTXT(volMsg);

    StaticJsonDocument<128> flowDoc;
    flowDoc["type"] = "flow";
    flowDoc["val"] = state.currentFlow;
    
    String flowMsg;
    serializeJson(flowDoc, flowMsg);
    webSocket.broadcastTXT(flowMsg);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_CONNECTED) {
        Serial.printf("[WS] Client #%u connected\n", num);
    } else if (type == WStype_DISCONNECTED) {
        Serial.printf("[WS] Client #%u disconnected\n", num);
    } else if (type == WStype_TEXT) {
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
            broadcastStatus();
        } else if (text.startsWith("setTarget:")) {
            if (state.relayActive) return;
            float newTarget = text.substring(10).toFloat();
            if (newTarget > state.accumulatedVolume) {
                state.volumeTarget = newTarget;
                state.targetReached = false;
                broadcastStatus();
            }
        } else if (text == "resetBatch") {
            if (state.relayActive) return;
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

void syncTime() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Serial.print("[TIME] Syncing time...");
    while (time(nullptr) < 1000 * 1000) {
        esp_task_wdt_reset();
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Time synced!");
}

void messageHandler(char* topic, byte* payload, unsigned int length) {
    String msg = "";
    for (int i = 0; i < length; i++) msg += (char)payload[i];
    if (msg == "relayOn") { state.relayActive = true; digitalWrite(RELAY_PIN, HIGH); }
    else if (msg == "relayOff") { state.relayActive = false; digitalWrite(RELAY_PIN, LOW); }
}

void connectToMQTT() {
    Serial.printf("\n[MQTT] Connecting to %s:%d...\n", MQTT_BROKER, MQTT_PORT);
    
    // Attempt connection
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
        Serial.println("[MQTT] Success! Connected to Mosquitto.");
        client.subscribe(MQTT_SUB_TOPIC);
    } else {
        Serial.printf("[MQTT] Connection Failed, rc=%d\n", client.state());
    }
}

void mqttTask(void * pvParameters) {
    esp_task_wdt_add(NULL); 
    Serial.println("[TASK] MQTTTask heartbeat on Core 0");
    unsigned long lastPub = 0;
    
    for(;;) {
        esp_task_wdt_reset(); 
        if (WiFi.status() == WL_CONNECTED) {
            if (!client.connected()) {
                connectToMQTT();
                vTaskDelay(pdMS_TO_TICKS(10000));
            } else {
                client.loop();

                // Periodic Publishing (Every 5 seconds)
                if (millis() - lastPub > 5000) {
                    lastPub = millis();
                    
                    StaticJsonDocument<200> doc;
                    doc["flow"] = state.currentFlow;
                    doc["volume"] = state.accumulatedVolume;
                    doc["relay"] = state.relayActive;
                    doc["target"] = state.volumeTarget;

                    char buffer[200];
                    serializeJson(doc, buffer);
                    
                    if (client.publish(MQTT_PUB_TOPIC, buffer)) {
                        Serial.printf("[MQTT] Published to [%s]: %s\n", MQTT_PUB_TOPIC, buffer);
                    } else {
                        Serial.printf("[MQTT] Publish FAILED to [%s]\n", MQTT_PUB_TOPIC);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void setup() {
    Serial.begin(115200);
    delay(500); 
    Serial.printf("\n\n--- ESP32 SYSTEM START v%s ---\n", FIRMWARE_VERSION);
    
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL); 

    preferences.begin("flow", false);
    state.accumulatedVolume = preferences.getFloat("lastVol", 0);

    pinMode(RELAY_PIN, OUTPUT);
    pinMode(VALVE_PIN, OUTPUT);
    analogReadResolution(12);

    Serial.printf("[WIFI] SSID: %s\n", ssid);
    WiFi.setTxPower(WIFI_POWER_11dBm); // Reduce power to prevent brownouts
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        esp_task_wdt_reset();
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WIFI] Connected OK");
        syncTime();
    } else {
        Serial.println("\n[WIFI] Connection FAILED (Timeout)");
    }

    server.on("/", handleRoot);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    // MQTT Setup
    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(messageHandler);
    client.setBufferSize(2048);
    client.setKeepAlive(60);

    // Task Spawning
    xTaskCreatePinnedToCore(flowControlTask, "FlowTask", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(mqttTask, "MQTTTask", 16384, NULL, 3, NULL, 0);

    Serial.println("[SYSTEM] Setup Sequence Complete.");
}

void loop() {
    esp_task_wdt_reset();
    server.handleClient();
    webSocket.loop();

    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 2000) {
        lastUpdate = millis();
        broadcastStatus();
        
        static unsigned long lastSave = 0;
        if (millis() - lastSave > 30000) {
            lastSave = millis();
            saveVolumeToNVS();
        }
    }
}
