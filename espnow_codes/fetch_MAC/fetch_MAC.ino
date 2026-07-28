#include <WiFi.h>
#include <esp_wifi.h>

// Custom valid MAC Address
uint8_t customMAC[] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};

void setup() {
  Serial.begin(115200);
  
  // 1. Initialize Wi-Fi
  WiFi.mode(WIFI_STA);
  delay(100);

  // 2. Override the MAC address
  esp_err_t result = esp_wifi_set_mac(WIFI_IF_STA, customMAC);
  
  if (result == ESP_OK) {
    Serial.println("Custom MAC successfully set!");
  } else {
    Serial.println("Error setting MAC! Check invalid byte rules.");
  }

  // 3. Verify the new MAC
  Serial.print("Current MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {}