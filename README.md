# Authento: RFID Attendance System

Authento is a real-time, RFID-based attendance tracking system built with an Arduino R4 WiFi, a Python backend server, and a web-based dashboard. It scans RFID cards using an MFRC522 reader and logs attendance instantly to a local server.

## 🌟 Features
- **Real-Time Tracking**: Attendance data is streamed directly to a web dashboard using Server-Sent Events (SSE).
- **Standalone Arduino**: The Arduino connects to your WiFi and communicates with the server over HTTP, meaning no USB connection is needed during operation.
- **CSV Logging**: All scans are permanently logged to a local CSV file for easy exporting and auditing.
- **User Mapping**: Easily map RFID UIDs to user names within the server code.
- **Lightweight Backend**: Runs entirely on Python's built-in libraries (no third-party dependencies required like Flask or Django).

## 🛠️ Hardware Requirements
- Arduino UNO R4 WiFi (or any ESP8266/ESP32 based board with minor code tweaks)
- MFRC522 RFID Reader
- RFID Cards/Tags
- Jumper wires and Breadboard

## 💻 Software Requirements
- Arduino IDE
- MFRC522 Library (by GithubCommunity) installed in Arduino IDE
- Python 3.x

## 🚀 Setup & Installation

### 1. Server Setup (Python)
The backend server receives the scans, logs them, and hosts the real-time web dashboard.

1. Navigate to the `server` directory.
2. The server runs on standard Python libraries. No `pip install` is necessary.
3. Open `server.py` and modify the `KNOWN_USERS` dictionary if you want to map specific RFID tags to user names.
4. Run the server:
   ```bash
   python server.py
   ```
5. The server will start on `http://localhost:3000`. Note your laptop's local IP address (e.g., `192.168.1.100`), as you will need it for the Arduino code.
6. Open a web browser and go to `http://localhost:3000` to view the live dashboard.

### 2. Hardware Setup (Arduino)
1. Wire the MFRC522 reader to the Arduino R4 WiFi via SPI.
   - **SDA (SS)**: Pin 10
   - **SCK**: Pin 13
   - **MOSI**: Pin 11
   - **MISO**: Pin 12
   - **IRQ**: Not connected
   - **GND**: GND
   - **RST**: Pin 9
   - **3.3V**: 3.3V (Do not connect to 5V!)

2. Open `arduino_rfid_attendance/arduino_rfid_attendance.ino` in the Arduino IDE.
3. Update the configuration section at the top of the file:
   ```cpp
   char ssid[] = "YOUR_WIFI_SSID";
   char pass[] = "YOUR_WIFI_PASSWORD";
   char serverAddress[] = "YOUR_LAPTOP_IP"; // Must be on the same network
   ```
4. Upload the code to your Arduino.
5. Open the Serial Monitor at `115200` baud to ensure it successfully connects to WiFi.
6. Scan a tag!

## 📡 API Endpoints

- `POST /api/scan`: Receives JSON payload `{"uid": "XXXXXX"}` from the Arduino.
- `GET /api/events`: SSE stream endpoint for real-time dashboard updates.

## 📄 License
This project is open-source and available for educational and personal use.
