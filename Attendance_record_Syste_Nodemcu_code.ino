#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <U8g2lib.h>  // Include the U8g2 library for OLED display

#define WIFI_SSID "123456789"
#define WIFI_PASSWORD "123456789"
#define API_KEY "AIzaSyBwDnlUXdTESqvAnohLLUPdBjTo9dpLHI0"
#define DATABASE_URL "https://srgec-c5798-default-rtdb.firebaseio.com/"

// Define OLED screen settings
#define SDA_PIN 4  // D2 pin (GPIO4) for SDA
#define SCL_PIN 5  // D1 pin (GPIO5) for SCL
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SDA_PIN, SCL_PIN, U8X8_PIN_NONE);  // Use I2C interface

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

String receivedUID = "";  // Variable to store the received UID

void setup() {
  // Open serial communications and wait for the port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize the OLED display
  u8g2.begin();
  u8g2.clearBuffer();  // Clear the internal buffer
  u8g2.setFont(u8g2_font_ncenB08_tr); // Set the font

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Auth successful");
    signupOK = true;
  } else {
    Serial.printf("Firebase Auth failed: %s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Check if there's data available on the serial port
  if (Serial.available()) {
    receivedUID = Serial.readStringUntil('\n');  // Read the incoming UID (until newline)
    receivedUID.trim();                          // Remove any extra whitespace or newline characters
    Serial.println("Received UID: " + receivedUID); // Print the received UID to serial monitor

    // Display UID on OLED screen
    u8g2.clearBuffer();  // Clear previous screen content
    u8g2.setCursor(0, 20);  // Set the cursor position
    u8g2.print("Received UID: ");
    u8g2.setCursor(0, 40);  // Move cursor to the next line
    u8g2.print(receivedUID);

    if (receivedUID == "INVALID_UID") {
      // Access Denied for invalid UID
      u8g2.setCursor(0, 60);  // Move cursor to the next line
      u8g2.print("Access Denied");
    } else {
      // Access Granted for valid UID
      u8g2.setCursor(0, 60);  // Move cursor to the next line
      u8g2.print("Access Granted");
    }

    u8g2.sendBuffer();  // Send the buffer to the display
    
      // Send UID to Firebase if valid
      if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();

        if (receivedUID != "") {
          if (Firebase.RTDB.setString(&fbdo, "mainbucket/Received_UID/01", receivedUID)) {
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
            Serial.println("Data sent successfully.");
          } else {
            Serial.println("Failed to send UID data. Reason: " + fbdo.errorReason());
          }
        }
      }
    }
  }
