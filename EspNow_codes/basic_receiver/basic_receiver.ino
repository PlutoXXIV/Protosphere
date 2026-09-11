#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

// Custom valid MAC Address
const uint8_t customMAC[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};

// 1. Callback function triggered automatically when data arrives
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  Serial.print("Received message: ");
  Serial.write(incomingData, len); // Print raw bytes directly as text
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  
  // 1. Initialize Wi-Fi
  WiFi.mode(WIFI_STA);
  delay(100);

  // 2. Override the MAC address
  esp_err_t result = esp_wifi_set_mac(WIFI_IF_STA, customMAC);
  delay(1000);

  if (result == ESP_OK) {
    Serial.println("Custom MAC successfully set!");
  } else {
    Serial.println("Error setting MAC! Check invalid byte rules.");
  }

  // 3. Verify the new MAC
  Serial.print("Current MAC Address: ");
  Serial.println(WiFi.macAddress());

  esp_now_init();      // Start ESP-NOW
  delay(1000);
  esp_now_register_recv_cb(OnDataRecv);
  
}

void loop() {}