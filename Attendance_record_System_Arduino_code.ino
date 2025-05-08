#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

constexpr uint8_t RST_PIN = 9;  // Configurable, see typical pin layout
constexpr uint8_t SS_PIN = 10;  // Configurable, see typical pin layout
MFRC522 rfid(SS_PIN, RST_PIN);  // Create instance of the RFID class

SoftwareSerial espSerial(5, 6); // RX, TX pins for communication with NodeMCU

// Array of registered RFID tags
String registeredCards[] = {
  "93B55635",  // Registered Card 1
  "334E1B35",  // Registered Card 2
  "939B5435"   // Registered Card 3
};

const int numRegisteredCards = sizeof(registeredCards) / sizeof(registeredCards[0]);

void setup() {
  Serial.begin(9600);    // Start serial communication
  espSerial.begin(115200); // Initialize SoftwareSerial communication (for NodeMCU)
  SPI.begin();           // Initialize SPI bus for RFID
  rfid.PCD_Init();       // Initialize MFRC522
  Serial.println("System Ready! Place your card near the reader...");
}

void loop() {
  // Check for new RFID card
  if (!rfid.PICC_IsNewCardPresent()) return;

  if (rfid.PICC_ReadCardSerial()) {
    String tag = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      tag += String(rfid.uid.uidByte[i], HEX); // Convert UID to string
    }
    tag.toUpperCase(); // Ensure the tag is in uppercase
    Serial.println("Detected Card UID: " + tag); // Print UID to serial monitor

    // Check if the detected card UID matches any registered card
    bool isRegistered = false;
    for (int i = 0; i < numRegisteredCards; i++) {
      if (registeredCards[i] == tag) {
        isRegistered = true;
        break;
      }
    }

    // If card is registered, mark attendance, else deny access
    if (isRegistered) {
      Serial.println("Attendance Marked for Card UID: " + tag);
      espSerial.println(tag); // Send attendance info to NodeMCU
    } else {
      Serial.println("Access Not Allowed! Unregistered Card.");
      espSerial.println("Unregistered Card."); // Deny access to NodeMCU
    }

    rfid.PICC_HaltA();       // Stop reading
    rfid.PCD_StopCrypto1();  // Stop encryption
  }
}
