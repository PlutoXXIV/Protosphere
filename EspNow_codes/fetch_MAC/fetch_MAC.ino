#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Wi-Fi must be initialized to read the MAC address
  WiFi.mode(WIFI_STA);
  
  Serial.println("\n--- ESP32 MAC Address Finder ---");
  Serial.print("Factory MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // Nothing to do here
}
