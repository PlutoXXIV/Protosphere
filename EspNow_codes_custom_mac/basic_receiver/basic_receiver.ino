#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// Define your fixed MAC address
uint8_t fixedMac[] = {0x24, 0x0A, 0xC4, 0x86, 0x5A, 0x0C};

// Callback function triggered when data arrives
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  Serial.print("Received message: ");
  Serial.write(incomingData, len);  // Print raw bytes as text
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n--- ESP-NOW Receiver with Fixed MAC ---");

  // 1. Initialize Wi-Fi in Station Mode (required for ESP-NOW)
  WiFi.mode(WIFI_STA);

  // 2. Set the custom MAC address
  esp_err_t result = esp_wifi_set_mac(WIFI_IF_STA, fixedMac);
  if (result == ESP_OK) {
    Serial.println("[SUCCESS] Custom MAC address assigned successfully!");
  } else {
    Serial.printf("[ERROR] Failed to set MAC address. Error: %d\n", result);
  }

  // 3. Read back the active MAC address
  uint8_t activeMac[6];
  esp_wifi_get_mac(WIFI_IF_STA, activeMac);

  Serial.printf("Active MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                activeMac[0], activeMac[1], activeMac[2],
                activeMac[3], activeMac[4], activeMac[5]);

  // 4. Initialize ESP-NOW
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP-NOW initialized successfully.");
  } else {
    Serial.println("Error initializing ESP-NOW!");
    return;
  }

  // 5. Register the receive callback
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Receiver ready. Waiting for data...");
}

void loop() {
  // Empty – reception is handled by the callback
}