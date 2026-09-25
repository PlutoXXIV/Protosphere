# Wireless quiz - ESP32 setup guide

10 ESP32 boards total: 1 **gateway** + 9 **nodes**. Each node has a 16x2
I2C LCD and an IR sensor.

## Wiring (per node, same on the gateway minus the LCD/IR)

- LCD SDA -> GPIO21, LCD SCL -> GPIO22 (ESP32 default I2C pins)
- IR sensor OUT -> GPIO4, plus VCC/GND
- The gateway only needs USB power/data to your PC - no LCD or IR sensor
  required on it (though it's fine if that board happens to have them too,
  the gateway firmware just won't use them).

## Libraries to install (Arduino IDE > Library Manager)

- `LiquidCrystal_I2C` (nodes only)
- `ArduinoJson` v6.x (nodes only)
- Board package: "esp32 by Espressif Systems" (Boards Manager) - gives you
  WiFi.h / esp_now.h / esp_wifi.h for both sketches

## Flashing

1. Flash `gateway/gateway.ino` to one ESP32. Leave it connected to your PC
   over USB - that's the serial link the Python script uses.
2. Flash `node/node.ino` to each of the other 9 boards, changing only
   `#define NODE_ID "esp1"` to `"esp2"`, `"esp3"`, ... `"esp9"` before each
   upload. Everything else in the file stays the same.
3. If an LCD stays blank, its I2C address probably isn't `0x27` - run an
   "I2C scanner" example sketch to find the real address and update
   `LCD_ADDR` in `node.ino`.
4. If a node's answers come out flipped (true when you'd expect false),
   set `IR_ACTIVE_LOW` to `false` in that node's code and reflash.

## Running the quiz

```
pip install pyserial
python quiz_server.py --port COM5          # Windows, or /dev/ttyUSB0 on Linux/Mac
```

Type a question and hit Enter to broadcast it (or pass `--questions
questions.txt` with one question per line to run through a prepared set).
Each node shows the question on its LCD with a live countdown. Answer by
covering the IR sensor with your hand (true) or leaving it clear (false) -
what the sensor reads right when the timer hits zero is what gets sent in.

You'll see a live results line like:

```
esp1 - true  esp2 - false  esp3 - ...  esp4 - true  ...
```

`...` means that node hasn't answered yet. Press Enter (or the script
auto-advances if you loaded a question file) whenever you're ready for the
next question.

## How the network works

- Gateway and nodes all pin themselves to WiFi channel 1 and talk over
  **ESP-NOW** - no router needed, works anywhere.
- The gateway doesn't parse anything: it just relays whatever line the PC
  sends out over ESP-NOW broadcast, and relays whatever any node sends it
  straight back over serial. All the logic lives in the PC script and the
  node firmware.
- Nodes learn the gateway's MAC address automatically from the first
  question they receive, so you never have to hardcode MAC addresses
  anywhere.
