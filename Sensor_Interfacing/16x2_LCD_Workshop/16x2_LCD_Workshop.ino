#include <Wire.h>

const int sdaPin = 21;
const int sclPin = 22;

uint8_t lcdAddress = 0;

const uint8_t LCD_BACKLIGHT = 0x08;
const uint8_t LCD_ENABLE    = 0x04;
const uint8_t LCD_RS        = 0x01;

void lcdWrite(uint8_t data) {
  Wire.beginTransmission(lcdAddress);
  Wire.write(data);
  Wire.endTransmission();
}

void lcdPulseEnable(uint8_t data) {
  lcdWrite(data | LCD_ENABLE);
  delayMicroseconds(1);
  lcdWrite(data & ~LCD_ENABLE);
  delayMicroseconds(50);
}

void lcdSend(uint8_t value, uint8_t mode) {
  uint8_t high = (value & 0xF0) | mode | LCD_BACKLIGHT;
  uint8_t low  = ((value << 4) & 0xF0) | mode | LCD_BACKLIGHT;

  lcdPulseEnable(high);
  lcdPulseEnable(low);
}

void lcdCommand(uint8_t command) {
  lcdSend(command, 0x00);
}

void lcdPrint(const char *text) {
  while (*text) {
    lcdSend(*text++, LCD_RS);
  }
}

void lcdSetCursor(int col, int row) {
  const uint8_t rowAddress[] = {0x00, 0x40};
  lcdCommand(0x80 | rowAddress[row] | col);
}

bool findLCD() {
  const uint8_t ranges[][2] = {
    {0x20, 0x27},
    {0x38, 0x3F}
  };

  for (auto &range : ranges) {
    for (uint8_t address = range[0]; address <= range[1]; address++) {
      Wire.beginTransmission(address);

      if (Wire.endTransmission() == 0) {
        lcdAddress = address;

        Serial.print("LCD found at 0x");
        if (address < 16) Serial.print("0");
        Serial.println(address, HEX);

        return true;
      }
    }
  }

  return false;
}

void lcdInit() {
  delay(50);

  // Proper HD44780 startup sequence
  lcdPulseEnable(0x30);
  delay(5);

  lcdPulseEnable(0x30);
  delayMicroseconds(150);

  lcdPulseEnable(0x30);
  delayMicroseconds(150);

  lcdPulseEnable(0x20);
  delayMicroseconds(150);

  lcdCommand(0x28);  // 4-bit, 2-line mode
  lcdCommand(0x08);  // Display off
  lcdCommand(0x01);  // Clear display
  delay(2);
  lcdCommand(0x06);  // Cursor moves right
  lcdCommand(0x0C);  // Display on, cursor off
}

void setup() {
  Serial.begin(115200);

  Wire.begin(sdaPin, sclPin);

  if (!findLCD()) {
    Serial.println("No I2C LCD found");
    return;
  }

  lcdInit();

  lcdSetCursor(0, 0);
  lcdPrint("PROTOSPHERE");

  lcdSetCursor(0, 1);
  lcdPrint("ESP32 + I2C");

  Serial.println("LCD initialized");
}

void loop() {
  if (lcdAddress == 0) {
    delay(1000);
    return;
  }

  delay(2000);

  lcdCommand(0x01);
  delay(2);

  lcdSetCursor(0, 0);
  lcdPrint("Embedded");

  lcdSetCursor(0, 1);
  lcdPrint("Systems & IoT");

  Serial.println("Embedded Systems & IoT");

  delay(2000);

  lcdCommand(0x01);
  delay(2);

  lcdSetCursor(0, 0);
  lcdPrint("I2C LCD");

  lcdSetCursor(0, 1);
  lcdPrint("16x2 Display");

  Serial.println("I2C LCD - 16x2 Display");
}
