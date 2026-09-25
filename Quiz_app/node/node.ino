/*
  Wireless Quiz - NODE firmware (v3 - scrolling marquee for long questions)
  ---------------------------------------------------------------------------
  Flash this onto each of the 9 node ESP32s. Change NODE_ID below to a
  unique label ("esp1".."esp9") before flashing each board.

  Questions that fit in 16 characters display normally. Longer questions
  scroll continuously across row 1 (a marquee) instead of being split
  across both rows. Row 2 is reserved for the node label + countdown.

  Answer rule: hold/cover the IR sensor while thinking. Whatever the
  sensor reads at the moment the answer window ends is the answer -
  covered = TRUE, clear = FALSE.

  DEBUGGING: plug a node into USB and open its Serial Monitor at
  115200 baud (separately from running quiz_server.py - only one
  program can own a serial port at a time). You should see, in order:
    [espN] ready
    [espN] raw packet (NN bytes): {...}          <- confirms it heard the broadcast
    [espN] question qid=1 window=8 text="..."    <- confirms JSON parsed correctly
    [espN] IR raw=1 -> covered=false              <- confirms what the sensor read
    [espN] sent answer: {...}

  Wiring:
    - I2C LCD (16x2):  SDA -> GPIO21, SCL -> GPIO22
    - IR sensor OUT -> IR_PIN (default GPIO4)

  Required libraries: LiquidCrystal_I2C, ArduinoJson (v6.x)
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
#define MAX_PACKET_LEN 250      // ESP-NOW's hard payload cap
#define SCROLL_INTERVAL_MS 400  // lower = faster scroll
#define SCROLL_GAP "   "        // blank gap shown between loops of the marquee

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

uint8_t gatewayMac[6] = {0};
bool haveGatewayMac = false;

// --- Handoff between the ESP-NOW callback (WiFi task) and loop() (main
// task). Keep the callback itself tiny - just copy bytes - and do all
// real work (JSON parsing, LCD writes) in loop().
volatile bool packetPending = false;
uint8_t pendingData[MAX_PACKET_LEN];
volatile int pendingLen = 0;
uint8_t pendingSrcMac[6];

volatile bool questionActive = false;
int currentQid = -1;
unsigned long questionStartMs = 0;
unsigned long windowMs = 0;
bool answered = true;

// --- Marquee state (row 0 only) ---
bool marqueeNeeded = false;  // false if the text already fits in LCD_COLS
String marqueeDoubled;       // "text + gap" repeated twice, for seamless wraparound
int marqueeLoopLen = 0;      // length of one loop (text + gap)
int marqueePos = 0;
unsigned long lastScrollMs = 0;

void beginQuestionDisplay(const String &text) {
  lcd.clear();

  if ((int)text.length() <= LCD_COLS) {
    marqueeNeeded = false;
    lcd.setCursor(0, 0);
    lcd.print(text);
  } else {
    marqueeNeeded = true;
    String loopText = text + SCROLL_GAP;
    marqueeLoopLen = loopText.length();
    marqueeDoubled = loopText + loopText;
    marqueePos = 0;
    lastScrollMs = millis();
    lcd.setCursor(0, 0);
    lcd.print(marqueeDoubled.substring(0, LCD_COLS));
  }

  lcd.setCursor(0, 1);
  lcd.print(NODE_ID);
}

void updateMarquee() {
  if (!marqueeNeeded || !questionActive) return;
  unsigned long now = millis();
  if (now - lastScrollMs < SCROLL_INTERVAL_MS) return;
  lastScrollMs = now;

  marqueePos = (marqueePos + 1) % marqueeLoopLen;
  lcd.setCursor(0, 0);
  lcd.print(marqueeDoubled.substring(marqueePos, marqueePos + LCD_COLS));
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
  Serial.print("[" NODE_ID "] sent answer: ");
  Serial.println(buf);
}

// Keep this callback minimal: just stash the packet for loop() to handle.
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len > MAX_PACKET_LEN) len = MAX_PACKET_LEN;
  memcpy(pendingData, data, len);
  pendingLen = len;
  memcpy(pendingSrcMac, info->src_addr, 6);
  packetPending = true;
}

void handlePendingPacket() {
  if (!packetPending) return;

  uint8_t buf[MAX_PACKET_LEN];
  int len = pendingLen;
  memcpy(buf, pendingData, len);
  uint8_t srcMac[6];
  memcpy(srcMac, pendingSrcMac, 6);
  packetPending = false;

  Serial.print("[" NODE_ID "] raw packet (");
  Serial.print(len);
  Serial.print(" bytes): ");
  Serial.write(buf, len);
  Serial.println();

  if (!haveGatewayMac) {
    memcpy(gatewayMac, srcMac, 6);
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, gatewayMac, 6);
    peer.channel = ESPNOW_CHANNEL;
    peer.encrypt = false;
    esp_now_add_peer(&peer);
    haveGatewayMac = true;
    Serial.println("[" NODE_ID "] learned gateway MAC, registered as peer");
  }

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, buf, len);
  if (err) {
    Serial.print("[" NODE_ID "] JSON parse failed: ");
    Serial.println(err.c_str());
    return;
  }

  int qid = doc["qid"] | -1;
  const char *text = doc["text"] | "";
  int window = doc["window"] | 8;
  if (qid < 0) {
    Serial.println("[" NODE_ID "] no qid in packet, ignoring");
    return;
  }

  Serial.print("[" NODE_ID "] question qid=");
  Serial.print(qid);
  Serial.print(" window=");
  Serial.print(window);
  Serial.print(" text=\"");
  Serial.print(text);
  Serial.println("\"");

  currentQid = qid;
  windowMs = (unsigned long)window * 1000UL;
  questionStartMs = millis();
  answered = false;
  questionActive = true;

  beginQuestionDisplay(String(text));
}

void setup() {
  Serial.begin(115200);
  delay(200);
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
    Serial.println("[" NODE_ID "] esp_now_init FAILED");
    while (true) delay(1000);
  }
  esp_now_register_recv_cb(onDataRecv);

  Serial.println("[" NODE_ID "] ready");
}

void loop() {
  handlePendingPacket();
  updateMarquee();

  if (!questionActive) return;

  unsigned long elapsed = millis() - questionStartMs;
  unsigned long remainingMs = (elapsed < windowMs) ? (windowMs - elapsed) : 0;

  static unsigned long lastTick = 0;
  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    lcd.setCursor(LCD_COLS - 3, 1);
    char cbuf[5];
    snprintf(cbuf, sizeof(cbuf), "%2lus", (remainingMs + 999) / 1000);
    lcd.print(cbuf);
  }

  if (!answered && elapsed >= windowMs) {
    int raw = digitalRead(IR_PIN);
    bool covered = IR_ACTIVE_LOW ? (raw == LOW) : (raw == HIGH);

    Serial.print("[" NODE_ID "] IR raw=");
    Serial.print(raw);
    Serial.print(" -> covered=");
    Serial.println(covered ? "true" : "false");

    sendAnswer(covered);
    answered = true;
    questionActive = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(NODE_ID);
    lcd.setCursor(0, 1);
    lcd.print(covered ? "Answered: TRUE" : "Answered: FALSE");
  }
}
