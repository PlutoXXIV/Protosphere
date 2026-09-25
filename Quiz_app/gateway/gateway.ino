/*
  Wireless Quiz - GATEWAY firmware
  --------------------------------
  Flash this onto ONE ESP32. It sits between your PC (over USB serial)
  and the 9 node ESP32s (over ESP-NOW).

  It does two simple jobs and does NOT parse any JSON itself:
    1. PC -> Gateway (Serial)   -> broadcast over ESP-NOW to all nodes
    2. Node -> Gateway (ESP-NOW) -> forward straight to Serial (PC)

  Serial settings: 115200 baud, newline-terminated lines.

  Required library: none beyond the ESP32 Arduino core
  ("esp32 by Espressif Systems" in Boards Manager) — WiFi.h, esp_now.h
  and esp_wifi.h all come with it.
*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <string.h>

// Broadcast address: every ESP-NOW peer listens on this
static uint8_t BROADCAST_ADDR[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Fixed WiFi channel every board (gateway + all 9 nodes) must share.
// ESP-NOW peers must be on the same channel to talk to each other.
#define ESPNOW_CHANNEL 1

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  // A node sent us an answer. Forward it to the PC exactly as received,
  // one JSON line per message.
  Serial.write(data, len);
  Serial.write('\n');
}

// Newer ESP32 Arduino cores (IDF 5.x) pass a wifi_tx_info_t* here instead of
// a raw MAC pointer. If you're on an older core (IDF 4.x) and this fails to
// compile the other way, change the first parameter back to
// "const uint8_t *mac".
void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  // Uncomment for debugging broadcast delivery:
  // Serial.print("{\"send_status\":\""); Serial.print(status == ESP_NOW_SEND_SUCCESS ? "ok" : "fail"); Serial.println("\"}");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  // make sure we're not trying to join a router
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("{\"error\":\"esp_now_init failed\"}");
    while (true) delay(1000);
  }

  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  // Register the broadcast address as a peer so we can send questions to it
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST_ADDR, 6);
  peer.channel = ESPNOW_CHANNEL;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.println("{\"status\":\"gateway ready\"}");
}

void loop() {
  // Read one line at a time from the PC and broadcast it verbatim.
  // Expected format from the PC, e.g.:
  //   {"qid":1,"text":"The sky is blue","window":8}
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0 && line.length() <= 240) {
      esp_now_send(BROADCAST_ADDR, (const uint8_t *)line.c_str(), line.length());
      Serial.print("{\"debug\":\"broadcast sent\",\"echo\":");
      Serial.print(line);
      Serial.println("}");
    }
  }
}
