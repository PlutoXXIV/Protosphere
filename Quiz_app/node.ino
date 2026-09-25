/*
  Wireless Quiz - NODE firmware
  ------------------------------
  Flash this onto each of the 9 node ESP32s. Change NODE_ID below to a
  unique label ("esp1".."esp9") before flashing each board — that's
  the only thing that needs to differ between the 9 copies.

  Answer rule: hold/cover the IR sensor while thinking. Whatever the
  sensor reads at the moment the answer window ends is the answer -
  covered = TRUE, clear = FALSE. There is no need to hold it for the
  whole window, just at the deadline.

  Wiring:
    - I2C LCD (16x2):  SDA -> GPIO21, SCL -> GPIO22 (ESP32 default I2C pins)
    - IR sensor OUT -> IR_PIN (default GPIO4)
      Most cheap IR obstacle sensors pull the output LOW when something
      is in front of them, HIGH when clear. That's what this code
      assumes - if your answers come out inverted, flip IR_ACTIVE_LOW.

  Required libraries (Library Manager):
    - LiquidCrystal_I2C (by Frank de Brabander, or a compatible fork)
    - ArduinoJson (by Benoit Blanchon, v6.x)

  If your LCD stays blank, your I2C address is probably not 0x27 -
  run an "I2C scanner" sketch (search that name) to find the real one
  and change LCD_ADDR below.
*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// ---- Per-board settings: change these before flashing each node ----
#define NODE_ID "esp1"
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2
#define IR_PIN 4
#define IR_ACTIVE_LOW true  // true = sensor output goes LOW when covered

#define ESPNOW_CHANNEL 1

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

uint8_t gatewayMac[6] = {0};
bool haveGatewayMac = false;

volatile bool questionActive = false;
int currentQid = -1;
unsigned long questionStartMs = 0;
unsigned long windowMs = 0;
bool answered = true;

void showQuestion(const String &text) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(text.substring(0, LCD_COLS));
  if ((int)text.length() > LCD_COLS) {
    lcd.setCursor(0, 1);
    int end = text.length() < 2 * LCD_COLS ? text.length() : 2 * LCD_COLS;
    lcd.print(text.substring(LCD_COLS, end));
  }
}

void sendAnswer(bool value) {
  StaticJsonDocument<128> doc;
  doc["node"] = NODE_ID;
  doc["qid"] = currentQid;
  doc["answer"] = value ? "true" : "false";
  char buf[128];
  size_t n = serializeJson(doc, buf, sizeof(buf));
  if (haveGatewayMac) {
    esp_now_send(gatewayMac, (const uint8_t *)buf, n);
  }
}

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  // Learn the gateway's MAC from whoever sends us a question
  if (!haveGatewayMac) {
    memcpy(gatewayMac, info->src_addr, 6);
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, gatewayMac, 6);
    peer.channel = ESPNOW_CHANNEL;
    peer.encrypt = false;
    esp_now_add_peer(&peer);
    haveGatewayMac = true;
  }

  StaticJsonDocument<200> doc;
  DeserializationError err = deserializeJson(doc, data, len);
  if (err) return;

  int qid = doc["qid"] | -1;
  const char *text = doc["text"] | "";
  int window = doc["window"] | 8;
  if (qid < 0) return;

  currentQid = qid;
  windowMs = (unsigned long)window * 1000UL;
  questionStartMs = millis();
  answered = false;
  questionActive = true;

  showQuestion(String(text));
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(NODE_ID);
  lcd.setCursor(0, 1);
  lcd.print("waiting...");

  pinMode(IR_PIN, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    lcd.clear();
    lcd.print("ESP-NOW init fail");
    while (true) delay(1000);
  }
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  if (!questionActive) return;

  unsigned long elapsed = millis() - questionStartMs;
  unsigned long remainingMs = (elapsed < windowMs) ? (windowMs - elapsed) : 0;

  // Live countdown in the bottom-right corner, updated once a second
  static unsigned long lastTick = 0;
  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    lcd.setCursor(LCD_COLS - 3, LCD_ROWS - 1);
    char buf[5];
    snprintf(buf, sizeof(buf), "%2lus", (remainingMs + 999) / 1000);
    lcd.print(buf);
  }

  if (!answered && elapsed >= windowMs) {
    // Time's up - sample the IR sensor right now
    int raw = digitalRead(IR_PIN);
    bool covered = IR_ACTIVE_LOW ? (raw == LOW) : (raw == HIGH);

    sendAnswer(covered);  // covered = TRUE, clear = FALSE
    answered = true;
    questionActive = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(NODE_ID);
    lcd.setCursor(0, 1);
    lcd.print(covered ? "Answered: TRUE" : "Answered: FALSE");
  }
}
