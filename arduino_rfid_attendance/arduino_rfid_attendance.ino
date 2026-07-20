#include <SPI.h>
#include <MFRC522.h>
#include <WiFiS3.h>

// --- WiFi Configuration ---
// Replace these with your actual WiFi credentials
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// --- Server Configuration ---
// Replace with the IP address of your laptop on the local network
// e.g., 192.168.1.100
char serverAddress[] = "YOUR_LAPTOP_IP"; 
int serverPort = 3000;

// --- RFID Configuration ---
#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
WiFiClient client;
int status = WL_IDLE_STATUS;

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect

  SPI.begin();			// Init SPI bus
  mfrc522.PCD_Init();		// Init MFRC522

  Serial.println(F("Welcome to the Arduino R4 WiFi RFID Attendance System!"));
  Serial.println(F("Connecting to WiFi..."));
  
  // Attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    if (status != WL_CONNECTED) {
      delay(5000); // Wait 5 seconds before retrying
    }
  }

  Serial.println("You're connected to the network");
  printWifiStatus();
  
  Serial.println(F("Scan an RFID tag to record attendance..."));
}

void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Get the UID as a string
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();

  Serial.print(F("Card Scanned UID: "));
  Serial.println(uidString);
  
  // Send the UID to the server
  sendToServer(uidString);

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
  
  // Small delay so it doesn't scan the same card multiple times instantly
  delay(2000); 
}

void sendToServer(String uid) {
  Serial.println("Connecting to server...");

  if (client.connect(serverAddress, serverPort)) {
    Serial.println("Connected to server! Sending data...");
    
    // Create the JSON payload
    String payload = "{\"uid\":\"" + uid + "\"}";
    
    // Make a HTTP POST request:
    client.println("POST /api/scan HTTP/1.1");
    client.print("Host: ");
    client.println(serverAddress);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(payload.length());
    client.println("Connection: close");
    client.println();
    client.println(payload);
    
    // Read the server response
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    }
    
    client.stop();
    Serial.println();
    Serial.println("Server disconnected.");
  } else {
    Serial.println("Connection to server failed!");
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
