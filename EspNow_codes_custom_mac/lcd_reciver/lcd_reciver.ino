#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 4);

#define ESPNOW_CHANNEL 1

// --------------------------------------------------
// CUSTOM MAC ADDRESS FOR THIS RECEIVER
// --------------------------------------------------

uint8_t receiverMAC[] = {
  0x02, 0x00, 0x00, 0x00, 0x00, 0x01
};


// --------------------------------------------------
// DATA STRUCTURE
// --------------------------------------------------

typedef struct {
  int irState;
} SensorData;

SensorData receivedData;


// --------------------------------------------------
// RECEIVE CALLBACK
// --------------------------------------------------

void onDataReceive(const esp_now_recv_info_t *info,
                   const uint8_t *data,
                   int len)
{
  if (len != sizeof(SensorData)) {
    return;
  }

  memcpy(&receivedData, data, sizeof(receivedData));

  Serial.print("Received IR state: ");
  Serial.println(receivedData.irState);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("REMOTE SENSOR");

  lcd.setCursor(0, 1);

  if (receivedData.irState == 1) {
    lcd.print("OBJECT DETECTED");
  }
  else {
    lcd.print("NO OBJECT");
  }

  lcd.setCursor(0, 3);
  lcd.print("ESP-NOW RX");
}


void setup()
{
  Serial.begin(115200);

  // --------------------------------------------------
  // START WIFI
  // --------------------------------------------------

  WiFi.mode(WIFI_STA);

  // --------------------------------------------------
  // SET OUR CUSTOM MAC ADDRESS
  // --------------------------------------------------

  esp_err_t result = esp_wifi_set_mac(WIFI_IF_STA, receiverMAC);

  if (result == ESP_OK) {
    Serial.println("Custom MAC address set successfully");
  }
  else {
    Serial.println("Failed to set MAC address");
  }

  // --------------------------------------------------
  // SET ESP-NOW CHANNEL
  // --------------------------------------------------

  esp_wifi_set_channel(
    ESPNOW_CHANNEL,
    WIFI_SECOND_CHAN_NONE
  );

  // --------------------------------------------------
  // PRINT MAC ADDRESS
  // --------------------------------------------------

  Serial.print("Receiver MAC: ");
  Serial.println(WiFi.macAddress());

  // --------------------------------------------------
  // LCD
  // --------------------------------------------------

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("ESP-NOW data...");

  // --------------------------------------------------
  // ESP-NOW
  // --------------------------------------------------

  if (esp_now_init() != ESP_OK) {

    Serial.println("ESP-NOW initialization failed");

    lcd.clear();
    lcd.print("ESP-NOW ERROR");

    return;
  }

  esp_now_register_recv_cb(onDataReceive);

  Serial.println("ESP-NOW receiver ready");
}


void loop()
{
}