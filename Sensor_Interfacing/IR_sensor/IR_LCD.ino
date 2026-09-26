#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define IR_PIN 15  // IR sensor output pin connected to GPIO 15

// Initialize the 16x2 LCD (I2C address 0x27 is standard; change to 0x3F if it doesn't display)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  pinMode(IR_PIN, INPUT);
  
  // Initialize I2C communication and LCD
  lcd.init();
  lcd.backlight();
  
  // Display initial startup message
  lcd.setCursor(0, 0);
  lcd.print("  ESP32 IR Sensor ");
  lcd.setCursor(0, 1);
  lcd.print("  Initializing... ");
  delay(2000);
  lcd.clear();
}

void loop() {
  int sensorValue = digitalRead(IR_PIN);

  lcd.setCursor(0, 0);
  lcd.print("Status:         "); // Clear line padding

  if (sensorValue == LOW) {
    // IR Obstacle detected
    lcd.setCursor(0, 1);
    lcd.print("Object Detected ");
  } else {
    // Path clear
    lcd.setCursor(0, 1);
    lcd.print("No Object       ");
  }

  delay(200); // Fast response time for sensor reading
}