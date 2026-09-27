#include <WiFi.h>
#include <esp_wifi.h>

// Define your fixed MAC address
uint8_t fixedMac[] = {0x24, 0x0A, 0xC4, 0x86, 0x5A, 0x0C};

void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("\n--- Setting Fixed MAC Address ---");

  // 1. Initialize Wi-Fi framework
  WiFi.mode(WIFI_STA);

  // 2. Overwrite the active Station interface MAC address
  esp_err_t result = esp_wifi_set_mac(WIFI_IF_STA, fixedMac);
  
  if (result == ESP_OK) {
    Serial.println("[SUCCESS] Custom MAC address assigned successfully!");
  } else {
    Serial.printf("[ERROR] Failed to set MAC address. Error: %d\n", result);
  }

  // 3. Read back from the LIVE network interface instead of the chip's eFuses
  uint8_t activeMac[6];
  esp_wifi_get_mac(WIFI_IF_STA, activeMac);

  // 4. Print the actual address the Wi-Fi card is transmitting with
  Serial.printf("Active Net MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                activeMac[0], activeMac[1], activeMac[2], 
                activeMac[3], activeMac[4], activeMac[5]);
}

void loop() {
  // Application code goes here
}
