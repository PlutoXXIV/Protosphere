#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>

// See WiFi_WebServer_Workshop.md for wiring, pin map, and design notes.

const char *ssid     = "ESP32_AP";
const char *password = "12345678";
WebServer server(80);

const int trigPin = 5;
const int echoPin = 18;
const int sdaPin  = 21;
const int sclPin  = 22;

uint8_t lcdAddress = 0;
bool lcdFound = false;
const uint8_t LCD_BACKLIGHT = 0x08, LCD_ENABLE = 0x04, LCD_RS = 0x01;

void lcdWrite(uint8_t d) { Wire.beginTransmission(lcdAddress); Wire.write(d); Wire.endTransmission(); }

void lcdPulse(uint8_t d) {
  lcdWrite(d | LCD_ENABLE);
  delayMicroseconds(1);
  lcdWrite(d & ~LCD_ENABLE);
  delayMicroseconds(50);
}

void lcdSend(uint8_t v, uint8_t mode) {
  lcdPulse((v & 0xF0) | mode | LCD_BACKLIGHT);
  lcdPulse(((v << 4) & 0xF0) | mode | LCD_BACKLIGHT);
}

void lcdCommand(uint8_t c) { lcdSend(c, 0x00); }
void lcdPrint(const char *s) { while (*s) lcdSend(*s++, LCD_RS); }

void lcdSetCursor(int col, int row) {
  const uint8_t rowAddr[] = {0x00, 0x40};
  lcdCommand(0x80 | rowAddr[row] | col);
}

bool findLCD() {
  const uint8_t ranges[][2] = {{0x20, 0x27}, {0x38, 0x3F}};
  for (auto &r : ranges) {
    for (uint8_t addr = r[0]; addr <= r[1]; addr++) {
      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0) { lcdAddress = addr; return true; }
    }
  }
  return false;
}

void lcdInit() {
  delay(50);
  lcdPulse(0x30); delay(5);
  lcdPulse(0x30); delayMicroseconds(150);
  lcdPulse(0x30); delayMicroseconds(150);
  lcdPulse(0x20); delayMicroseconds(150);
  lcdCommand(0x28); lcdCommand(0x08); lcdCommand(0x01);
  delay(2);
  lcdCommand(0x06); lcdCommand(0x0C);
}

// No-op if no LCD was found (avoids writing to reserved I2C address 0)
void lcdShowMessage(const String &msg) {
  if (!lcdFound) return;
  String line2 = msg.substring(0, 16);
  lcdCommand(0x01);
  delay(2);
  lcdSetCursor(0, 0);
  lcdPrint("Phone Message:");
  lcdSetCursor(0, 1);
  lcdPrint(line2.c_str());
}

long getDistance() {
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration == 0 ? -1 : duration * 0.034 / 2;
}

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Dashboard</title>
</head>
<body style="text-align:center;font-family:Arial">
  <h2>ESP32 Dashboard</h2>
  <h3>Live Sensor Data</h3>
  <h1><span id="distance">--</span> cm</h1>
  <hr>
  <h3>Control LCD</h3>
  <form action="/lcd" method="POST">
    <input type="text" name="msg" maxlength="16" required>
    <input type="submit" value="Send to LCD">
  </form>
  <script>
    setInterval(() => {
      fetch("/sensor").then(r => r.text()).then(d => {
        document.getElementById("distance").innerText = d;
      });
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

void handleHome()   { server.send(200, "text/html", webpage); }
void handleSensor() { long d = getDistance(); server.send(200, "text/plain", d >= 0 ? String(d) : "out of range"); }

void handleLCD() {
  lcdShowMessage(server.arg("msg"));
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Wire.begin(sdaPin, sclPin);
  lcdFound = findLCD();
  if (lcdFound) {
    lcdInit();
    lcdSetCursor(0, 0); lcdPrint("ESP32_AP");
    lcdSetCursor(0, 1); lcdPrint("192.168.4.1");
  } else {
    Serial.println("No I2C LCD found - continuing without it");
  }

  WiFi.softAP(ssid, password);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleHome);
  server.on("/sensor", handleSensor);
  server.on("/lcd", HTTP_POST, handleLCD);
  server.begin();
}

void loop() {
  server.handleClient();
}
