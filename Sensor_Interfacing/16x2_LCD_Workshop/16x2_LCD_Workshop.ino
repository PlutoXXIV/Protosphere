#include <Wire.h>

const int sdaPin = 21;
const int sclPin = 22;

const uint8_t lcdAddress = 0x27;

// Send one byte to the LCD through the I²C backpack
void lcdWrite(uint8_t data) {
  Wire.beginTransmission(lcdAddress);
  Wire.write(data);
  Wire.endTransmission();
}

// Toggle Enable to make the LCD read the sent nibble
void lcdPulseEnable(uint8_t data) {
  lcdWrite(data | 0x04);
  delayMicroseconds(1);
  lcdWrite(data & ~0x04);
  delayMicroseconds(50);
}

// Send a command or character in 4-bit mode
void lcdSend(uint8_t value, uint8_t mode) {
  uint8_t high = (value & 0xF0) | mode | 0x08;
  uint8_t low  = ((value << 4) & 0xF0) | mode | 0x08;

  lcdPulseEnable(high);
  lcdPulseEnable(low);
}

void lcdCommand(uint8_t command) {
  lcdSend(command, 0x00);
}

// Print a string character by character
void lcdPrint(const char *text) {
  while (*text) {
    lcdSend(*text++, 0x01);
  }
}

// Set cursor position: column 0-15, row 0-1
void lcdSetCursor(int col, int row) {
  const uint8_t rowAddress[] = {0x00, 0x40};
  lcdCommand(0x80 | (rowAddress[row] + col));
}

// Initialize the HD44780 LCD in 4-bit mode
void lcdInit() {
  delay(50);

  lcdWrite(0x30);
  delay(5);
  lcdWrite(0x30);
  delayMicroseconds(150);
  lcdWrite(0x30);
  lcdWrite(0x20);

  lcdCommand(0x28);  // 4-bit, 2-line mode
  lcdCommand(0x08);  // Display off
  lcdCommand(0x01);  // Clear display
  delay(2);
  lcdCommand(0x06);  // Move cursor right
  lcdCommand(0x0C);  // Display on, cursor off
}

void setup() {
  Serial.begin(115200);

  // ESP32 default I²C pins
  Wire.begin(sdaPin, sclPin);

  lcdInit();

  lcdSetCursor(0, 0);
  lcdPrint("PROTOSPHERE");

  lcdSetCursor(0, 1);
  lcdPrint("ESP32 + I2C");

  Serial.println("LCD initialized");
}

void loop() {
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
