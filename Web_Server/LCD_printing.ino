#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char *ssid = "ESP32";
const char *password = "12345678";
const int irPin = 4;   // IR sensor output pin (LOW = hand detected on most modules)

LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

String lastMessage = "Ready";
unsigned long lastSensorUpdate = 0;
const unsigned long sensorInterval = 500; // ms between IR refreshes

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  body{background:#0a1f44;color:#fff;font-family:'Arial Black',Arial,sans-serif;text-align:center;margin-top:40px;}
  h1{text-transform:uppercase;letter-spacing:4px;font-size:2.2em;color:#4fc3f7;text-shadow:2px 2px 6px rgba(0,0,0,.5);margin-bottom:5px;}
  h3{font-weight:normal;color:#cfd8dc;margin-top:0;}
  .card{background:#12305c;max-width:320px;margin:30px auto;padding:25px;border-radius:12px;box-shadow:0 4px 12px rgba(0,0,0,.4);}
  .sensor{background:#1c3d6e;padding:12px;border-radius:8px;color:#ff8a65;font-weight:bold;margin-bottom:20px;}
  input[type="text"]{width:85%;padding:10px;border-radius:6px;border:none;margin-bottom:15px;font-size:1em;}
  input[type="submit"]{background:#4fc3f7;color:#0a1f44;border:none;padding:10px 25px;border-radius:6px;font-weight:bold;font-size:1em;cursor:pointer;}
  input[type="submit"]:hover{background:#29b6f6;}
</style>
<body>
  <h1>Protosphere</h1>
  <h3>ESP32 LCD Control</h3>
  <div class="card">
    <div class="sensor" id="irStatus">Checking IR sensor...</div>
    <form action="/set_lcd" method="POST">
      <input type="text" name="msg" placeholder="Type text..." maxlength="16" required><br>
      <input type="submit" value="Send to Screen">
    </form>
  </div>
  <script>
    setInterval(() => {
      fetch('/ir_status').then(r => r.text()).then(d => document.getElementById('irStatus').innerText = d);
    }, 500);
  </script>
</body>
)rawliteral";

// Reads the IR pin and returns a status string
String readIRStatus() {
  return digitalRead(irPin) == LOW ? "Hand Detected" : "Hand Not Detected";
}

// Clears a row and prints text on it (avoids repeating this pattern everywhere)
void lcdPrintRow(int row, const String &text) {
  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
  lcd.print(text);
}

void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
}

void handleIRStatus() {
  server.send(200, "text/plain", readIRStatus());  // calls readIRStatus()
}

void handleLCDInput() {
  if (server.hasArg("msg")) {
    lastMessage = server.arg("msg");
    lcdPrintRow(0, lastMessage);  // calls lcdPrintRow() to update row 1
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  pinMode(irPin, INPUT);

  lcd.init();
  lcd.backlight();

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", IP);  // captive portal: any URL redirects here

  Serial.print("Open in browser: http://");
  Serial.println(IP);

  lcdPrintRow(0, "Connect Wi-Fi:");  // calls lcdPrintRow()
  lcdPrintRow(1, IP.toString());     // calls lcdPrintRow()

  server.on("/", HTTP_GET, handleRoot);              // calls handleRoot() on "/"
  server.on("/ir_status", HTTP_GET, handleIRStatus); // calls handleIRStatus() on "/ir_status"
  server.on("/set_lcd", HTTP_POST, handleLCDInput);  // calls handleLCDInput() on "/set_lcd"
  server.onNotFound(handleRoot);                     // calls handleRoot() for unknown URLs
  server.begin();

  delay(3000);
  lcdPrintRow(0, lastMessage);  // calls lcdPrintRow()
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (millis() - lastSensorUpdate >= sensorInterval) {
    lastSensorUpdate = millis();
    lcdPrintRow(1, readIRStatus());  // calls readIRStatus(), then lcdPrintRow()
  }
}