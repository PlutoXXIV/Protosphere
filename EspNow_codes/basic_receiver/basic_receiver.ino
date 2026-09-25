#include <WiFi.h>
#include <esp_now.h>

// Callback function triggered automatically when data arrives
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  Serial.print("Received message: ");
  Serial.write(incomingData, len); // Print raw bytes directly as text
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize Wi-Fi in Station Mode (Required for ESP-NOW)
  WiFi.mode(WIFI_STA);

  // Verify and print the factory MAC address
  Serial.print("Receiver Active MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP-NOW initialized successfully.");
  } else {
    Serial.println("Error initializing ESP-NOW!");
    return;
  }
  
  // Register the data reception callback function
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Left empty; callbacks handle data reception asynchronously in the background
}
