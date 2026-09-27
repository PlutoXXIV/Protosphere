#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// This device's fixed Station MAC address
uint8_t fixedMac[] = {0x2, 0x0A, 0xC4, 0x86, 0x5A, 0x0C};

// Peer MAC address (will be filled by user)
uint8_t peerAddress[6];

// Message buffers
char outgoingMessage[250];
char incomingMessage[250];

// Callback when data is received
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  // Copy and null-terminate, keeping within the destination buffer
  if (len < 0) return;
  size_t copyLen = (size_t)len;
  if (copyLen >= sizeof(incomingMessage)) {
    copyLen = sizeof(incomingMessage) - 1;
  }
  memcpy(incomingMessage, incomingData, copyLen);
  incomingMessage[copyLen] = '\0';

  Serial.print("\n>>> Received: ");
  Serial.println(incomingMessage);
  Serial.print("You: ");
}

// Callback when data is sent (ESP32 Arduino core 3.x / ESP-IDF 5.x)
void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? " [Sent OK]" : " [Send FAIL]");
  Serial.println();
  Serial.print("You: ");
}

// Helper: Convert MAC string "AA:BB:CC:DD:EE:FF" to byte array
bool parseMac(const char* macStr, uint8_t* macBytes) {
  int values[6];
  if (sscanf(macStr, "%x:%x:%x:%x:%x:%x",
             &values[0], &values[1], &values[2],
             &values[3], &values[4], &values[5]) == 6) {
    for (int i = 0; i < 6; i++) {
      macBytes[i] = (uint8_t)values[i];
    }
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=================================");
  Serial.println("   ESP-NOW Chat (Both Devices)");
  Serial.println("=================================");

  // Set Wi-Fi to Station mode before changing its active MAC
  WiFi.mode(WIFI_STA);

  // Assign the requested fixed MAC to the active Station interface
  esp_err_t result = esp_wifi_set_mac(WIFI_IF_STA, fixedMac);
  if (result != ESP_OK) {
    Serial.printf("[ERROR] Failed to set MAC address. Error: %d\n", result);
    return;
  }
  Serial.println("[SUCCESS] Custom MAC address assigned successfully!");

  // Read back the MAC from the live network interface
  uint8_t activeMac[6];
  result = esp_wifi_get_mac(WIFI_IF_STA, activeMac);
  if (result != ESP_OK) {
    Serial.printf("[ERROR] Failed to read active MAC address. Error: %d\n", result);
    return;
  }
  Serial.printf("Active Net MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                activeMac[0], activeMac[1], activeMac[2],
                activeMac[3], activeMac[4], activeMac[5]);
  Serial.println();

  // Ask user for the other device's MAC
  Serial.println("Enter the OTHER ESP's MAC address");
  Serial.println("Format: AA:BB:CC:DD:EE:FF");
  Serial.print("Peer MAC: ");

  // Wait until user enters a valid MAC
  while (true) {
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();

      if (parseMac(input.c_str(), peerAddress)) {
        Serial.println(input);
        Serial.println("MAC accepted!");
        break;
      } else {
        Serial.println("\nInvalid MAC format. Try again:");
        Serial.print("Peer MAC: ");
      }
    }
  }

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callbacks
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // Add peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("\nReady! Type a message and press Enter.");
  Serial.print("You: ");
}

void loop() {
  if (Serial.available()) {
    // Read the message typed by user
    size_t len = Serial.readBytesUntil('\n', outgoingMessage, sizeof(outgoingMessage) - 1);
    outgoingMessage[len] = '\0';

    // Remove trailing \r (Windows)
    if (len > 0 && outgoingMessage[len - 1] == '\r') {
      outgoingMessage[len - 1] = '\0';
      len--;
    }

    if (len > 0) {
      // Echo what we are sending
      Serial.println(outgoingMessage);

      // Send via ESP-NOW
      esp_err_t result = esp_now_send(peerAddress, (uint8_t *)outgoingMessage, len + 1);

      if (result != ESP_OK) {
        Serial.println("Error sending message");
        Serial.print("You: ");
      }
    } else {
      Serial.print("You: ");
    }
  }
}
