# 🌱 IoT-Based Smart Greenhouse with Cloud Logging & AI Consultation

## 📌 Project Overview

Traditional greenhouse monitoring systems output raw telemetry that is often difficult for non-technical users to interpret, lacking both historical data retention and actionable intelligence.

This project is an **IoT-enabled smart greenhouse platform** optimized for precision agriculture (specifically calibrated for *Cili* cultivation). It integrates edge computing, cloud data logging, and a Retrieval-Augmented Generation (RAG) AI agent. By combining a robust **Node-RED** orchestration layer with **Firebase** and the **ElevenLabs LLM API**, this system bridges the gap between raw hardware metrics and actionable human understanding, automatically translating environmental variables into natural-language botanical advice.

## ✨ Key Features

* **Real-Time Edge Telemetry:** Continuous monitoring of ambient temperature, humidity, and soil moisture via an ESP32 microcontroller.
* **Automated Thermal Regulation:** Local threshold logic actuates an electrically isolated exhaust fan when temperatures exceed critical levels (e.g., > 34°C to prevent blossom drop).
* **Permanent Cloud Logging:** Telemetry is routed via MQTT and formatted into JSON payloads for permanent historical retention in a Firebase Realtime Database.
* **AI Botanical Agent:** Anomalous environmental data triggers an external LLM API, generating expert-level, natural-language diagnostic alerts directly on the web dashboard.
* **EMI-Resistant Architecture:** Physical inductive loads (like cheap water pumps) were strategically excluded from this build to prevent electromagnetic interference (EMI) and ensure 100% sensor data integrity.

---

## 🛠️ System Architecture

### 1. Hardware Stack

* **Microcontroller:** ESP32 (NodeMCU)
* **Temperature/Humidity Sensor:** DHT22 (AM2302) for high accuracy (±0.5°C).
* **Soil Moisture Sensor:** Analog resistance-based sensor.
* **Actuator:** L9110 DC Exhaust Fan (12V) controlled via a 4-channel Relay Module.

### 2. Software & Cloud Stack

* **Firmware:** C++ (Arduino IDE)
* **Communication Protocol:** HiveMQ (MQTT Broker)
* **Middleware / Dashboard:** Node-RED (JavaScript)
* **Database:** Firebase Realtime Database (NoSQL)
* **Intelligence Layer:** ElevenLabs LLM API

---

## 🚀 How It Works (The Data Pipeline)

1. **Perception:** The ESP32 gathers readings from the DHT22 and soil moisture sensors.
2. **Transmission:** The ESP32 publishes the raw telemetry strings to a lightweight HiveMQ MQTT topic.
3. **Orchestration:** Node-RED subscribes to the MQTT topic, visualizes the data on a live dashboard, and formats the data into structured JSON objects.
4. **Data Persistence:** Node-RED executes an HTTP POST request, pushing the JSON payload to Firebase for permanent logging.
5. **AI Consultation:** If critical thresholds are breached (e.g., soil moisture drops below 20%), Node-RED passes the contextual data to the ElevenLabs API. The AI generates a text warning (e.g., *"Soil moisture is dangerously low. Roots are searching for hydration..."*) which is immediately displayed on the user dashboard.

### Example JSON Data Log

```json
{
  "timestamp": "16/05/2026, 14:32:10",
  "temperature": 32.2,
  "humidity": 77.0,
  "soil_moisture": 18,
  "fan_state": "ON"
}

```

---

## ⚙️ Setup & Installation

### Edge Hardware (ESP32)

1. Clone this repository and open the `.ino` file in the **Arduino IDE**.
2. Install the required libraries: `WiFi.h`, `PubSubClient.h`, and `DHT.h`.
3. Update the Wi-Fi credentials and HiveMQ broker details in the code.
4. Flash the code to the ESP32.

### Cloud Architecture (Node-RED & Firebase)

1. Import the `flows.json` file from this repository into your **Node-RED** workspace.
2. Configure the MQTT-in nodes to match your HiveMQ topic.
3. Set up a **Firebase Realtime Database** project. Update the Node-RED HTTP request nodes with your specific Firebase URL and authentication rules.
4. Insert your **ElevenLabs API Key** into the designated AI function node.
5. Click **Deploy** to launch the dashboard and begin data routing.

---

## 🔮 Future Recommendations

To scale this prototype for commercial farm deployment, the following enhancements are recommended:

1. **Isolated Autonomous Irrigation:** Reintroduce physical water pumps utilizing strict electrical isolation (optocouplers and decoupled power supplies) to prevent EMI corruption on the ESP32 logic circuit.
2. **Enterprise Orchestration:** Migrate the cloud routing layer from Node-RED to a dedicated workflow automation platform like **n8n** for advanced webhook management and multi-database branching.
3. **Predictive Machine Learning:** Utilize the historical datasets retained in Firebase to train a time-series forecasting model, shifting the system from reactive alerts to predictive agronomy.
