#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// ===== WIFI CREDENTIALS =====
const char* ssidList[] = {"B40Tegar", "BlinkBlink"};
const char* passList[] = {"MiskinSejati", "12341234"};
int wifiCount = 2;

// ===== MQTT CONFIGURATION =====
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Separate topics for easy tracking
const char* temp_topic = "biot/shaiful/temp";
const char* hum_topic  = "biot/shaiful/humidity";
const char* soil_topic = "biot/shaiful/soil";
const char* mode_topic = "biot/shaiful/mode";
const char* cmd_topic  = "biot/shaiful/greenhouse/cmd"; 
const char* fan_topic  = "biot/shaiful/fan_status";

WiFiClient espClient;
PubSubClient client(espClient);

// ===== SENSORS & ACTUATORS =====
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#define SOIL_PIN 34 

// Fan Pins (L9110)
#define INA 26
#define INB 27

// ===== SYSTEM STATES =====
String mode = "THRESHOLD"; 
String fanState = "OFF";   
unsigned long lastMsg = 0; 

// ===== FAN CONTROL =====
void fanOn() {
  digitalWrite(INA, HIGH);
  digitalWrite(INB, LOW);
  if (fanState != "ON") {
    fanState = "ON";
    Serial.println("--> Physical Action: Fan turned ON (Venting Heat)");
    client.publish(fan_topic, "ON");
  }
}

void fanOff() {
  digitalWrite(INA, LOW);
  digitalWrite(INB, LOW);
  if (fanState != "OFF") {
    fanState = "OFF";
    Serial.println("--> Physical Action: Fan turned OFF (Optimal Temp)");
    client.publish(fan_topic, "OFF");
  }
}

// ===== MQTT CALLBACK =====
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) { message += (char)payload[i]; }
  
  Serial.print("MQTT Received ["); Serial.print(topic); Serial.print("]: "); Serial.println(message);

  if (String(topic) == cmd_topic) {
    if (message == "mode_manual") {
      mode = "MANUAL";
      Serial.println("SYSTEM MODE: MANUAL");
      client.publish(mode_topic, "MANUAL");
    } 
    else if (message == "mode_threshold") {
      mode = "THRESHOLD";
      Serial.println("SYSTEM MODE: THRESHOLD (AUTO)");
      client.publish(mode_topic, "THRESHOLD");
    }

    // Manual overrides
    if (mode == "MANUAL") {
      if (message == "fan_on") fanOn();
      else if (message == "fan_off") fanOff();
    }
  }
}

void setup_wifi() {
  Serial.println("\nConnecting to WiFi...");
  WiFi.mode(WIFI_STA);
  for (int i = 0; i < wifiCount; i++) {
    Serial.print("Trying: "); Serial.println(ssidList[i]);
    WiFi.disconnect(true);
    delay(500);
    WiFi.begin(ssidList[i], passList[i]);

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 15) { delay(500); Serial.print("."); retry++; }
    if (WiFi.status() == WL_CONNECTED) { Serial.println("\nWiFi Connected!"); return; }
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-Shaiful-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected to HiveMQ!");
      client.subscribe(cmd_topic);
    } else {
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  analogReadResolution(12);       
  analogSetAttenuation(ADC_11db); 

  pinMode(INA, OUTPUT); 
  pinMode(INB, OUTPUT);
  fanOff(); 

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); 
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) setup_wifi();
  if (!client.connected()) reconnectMQTT();
  client.loop(); 

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int soilRaw = analogRead(SOIL_PIN);
    
    // Convert RAW ADC to Percentage (4095 = Dry, 1500 = Wet)
    int soil = constrain(map(soilRaw, 4095, 1500, 0, 100), 0, 100);

    if (!isnan(h) && !isnan(t)) {
      
      // Threshold mode logic optimized for Cili Plants
      if (mode == "THRESHOLD") {
        if (t > 34.0) fanOn();       // Prevent blossom drop
        else if (t < 31.0) fanOff(); // Optimal warm temperature
      }

      // Publish telemetry every 5 seconds
      client.publish(temp_topic, String(t).c_str());
      client.publish(hum_topic, String(h).c_str());
      client.publish(soil_topic, String(soil).c_str());
      client.publish(mode_topic, mode.c_str());
      
      Serial.printf("Sent Data -> T: %.2fC | H: %.2f%% | S: %d%% | M: %s\n", t, h, soil, mode.c_str());
    } else {
      Serial.println("Failed to read from DHT sensor!");
    }
    Serial.println("-------------------------");
  }
}