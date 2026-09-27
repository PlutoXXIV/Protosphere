#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// --------------------------------------------------
// IR SENSOR
// --------------------------------------------------

#define IR_PIN 27

// --------------------------------------------------
// ESP-NOW CHANNEL
// --------------------------------------------------

#define ESPNOW_CHANNEL 1

// --------------------------------------------------
// RECEIVER MAC ADDRESS
// --------------------------------------------------

// IMPORTANT:
// Replace this with the MAC address printed by
// your receiver ESP32.

uint8_t receiverMAC[] = {
  0x02, 0x00, 0x00, 0x00, 0x00, 0x01
};


// --------------------------------------------------
// Data structure
// --------------------------------------------------

// This must match the structure used by
// the receiver.

typedef struct {
  int irState;
} SensorData;

SensorData sensorData;


// --------------------------------------------------
// Send Callback
// --------------------------------------------------

// This function runs after ESP-NOW finishes
// attempting to send a packet.

void onDataSent(const wifi_tx_info_t *info,
                esp_now_send_status_t status)
{
  Serial.print("Send status: ");

  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("SUCCESS");
  }
  else {
    Serial.println("FAILED");
  }
}


void setup()
{
  Serial.begin(115200);

  // ------------------------------------------------
  // Configure IR sensor pin
  // ------------------------------------------------

  pinMode(IR_PIN, INPUT);

  // ------------------------------------------------
  // Start Wi-Fi
  // ------------------------------------------------

  WiFi.mode(WIFI_STA);

  // Use the same channel as the receiver.
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  Serial.println();
  Serial.println("Sender ESP32");

  Serial.print("Sender MAC: ");
  Serial.println(WiFi.macAddress());

  // ------------------------------------------------
  // Initialize ESP-NOW
  // ------------------------------------------------

  if (esp_now_init() != ESP_OK) {

    Serial.println("ESP-NOW initialization failed");

    return;
  }

  // ------------------------------------------------
  // Register send callback
  // ------------------------------------------------

  esp_now_register_send_cb(onDataSent);

  // ------------------------------------------------
  // Add receiver as a peer
  // ------------------------------------------------

  esp_now_peer_info_t peerInfo = {};

  // Copy receiver MAC address into peer structure.
  memcpy(peerInfo.peer_addr, receiverMAC, 6);

  // Use the same channel.
  peerInfo.channel = ESPNOW_CHANNEL;

  // Use the Station interface.
  peerInfo.ifidx = WIFI_IF_STA;

  // No encryption for this beginner example.
  peerInfo.encrypt = false;

  // Add receiver.
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {

    Serial.println("Failed to add receiver");

    return;
  }

  Serial.println("ESP-NOW sender ready");
}


void loop()
{
  // ------------------------------------------------
  // Read IR sensor
  // ------------------------------------------------

  int irValue = digitalRead(IR_PIN);

  // ------------------------------------------------
  // Store sensor value in our structure
  // ------------------------------------------------

  sensorData.irState = irValue;

  // ------------------------------------------------
  // Send the data
  // ------------------------------------------------

  esp_err_t result = esp_now_send(
    receiverMAC,
    (uint8_t *) &sensorData,
    sizeof(sensorData)
  );

  // Check whether the packet was accepted
  // for transmission.
  if (result != ESP_OK) {
    Serial.println("Error sending data");
  }

  // Send once every 500 ms.
  delay(500);
}