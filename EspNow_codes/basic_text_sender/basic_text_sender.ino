#include <esp_now.h>
#include <WiFi.h>

// Fixed Custom MAC Address of the Receiver ESP
uint8_t receiverAddress[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};

// Text message payload
char msg[] = "hello IoT";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Wi-Fi Station Mode is required

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) return;

  // Add Receiver as Peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  esp_now_add_peer(&peerInfo);
}

void loop() {
  // Send the message array
  esp_now_send(receiverAddress, (uint8_t *)msg, sizeof(msg));
  
  Serial.println("Sent: hello IoT");
  delay(2000); // Wait 2 seconds
}