# WiFi_WebServer_Workshop

ESP32 broadcasts its own WiFi network and hosts a webpage that lets a phone
send a message to the LCD and watch the live ultrasonic reading.

## Wiring

```
Ultrasonic VCC  -> ESP32 3V3
Ultrasonic GND  -> ESP32 GND
Ultrasonic TRIG -> GPIO 5
Ultrasonic ECHO -> GPIO 18

LCD SDA -> GPIO 21
LCD SCL -> GPIO 22
```

No LED is used in this sketch, so there's no GPIO21 conflict here. That
conflict only comes up if you also wire in the separate
`Ultrasonic_Sensor_Workshop` LED on the same board.

## Using it

1. Upload the sketch.
2. On your phone, connect to WiFi network `ESP32_AP`, password `12345678`.
3. Open `http://192.168.4.1` in a browser.
4. Type a message and tap "Send to LCD" - it appears on line 2 of the display.
5. The distance reading on the page updates every 2 seconds on its own.

## Design notes / fixes vs. the earlier draft

- **No external library.** The LCD is driven with the same hand-rolled
  `Wire.h` code used in `16x2_LCD_Workshop.ino`, not `LiquidCrystal_I2C.h`.
  That keeps this consistent with the rest of the repo, which intentionally
  needs nothing beyond the ESP32 board package - no extra Library Manager
  install for every kit before the event.
- **`findLCD()` auto-detects the I2C address** (scans `0x20-0x27` and
  `0x38-0x3F`) instead of hardcoding `0x27`, so it works on both PCF8574 and
  PCF8574A backpacks without anyone needing to know which one they have.
- **`/sensor` returns the real ultrasonic distance**, not a random number -
  it runs the same TRIG/ECHO/`pulseIn` sequence as the standalone ultrasonic
  sketch.
- **Message length is capped at 16 characters server-side** (`msg.substring(0,16)`),
  not just via the HTML `maxlength` attribute, which only stops the on-screen
  form and not a direct request to `/lcd`.
- **`lcdShowMessage()` no-ops if no LCD was detected at boot**, so a missing
  or miswired display can't cause a write to I2C address `0` (the reserved
  general-call address).
