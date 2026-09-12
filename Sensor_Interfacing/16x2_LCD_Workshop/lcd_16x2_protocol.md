Unlike a directly connected parallel LCD, an I²C LCD uses only two communication lines.

SDA = data line. It carries information between the ESP32 and the LCD.

SCL = clock line. It synchronizes the communication.

The ESP32 sends commands and display data through SDA while SCL controls the timing of the communication.

The LCD also has an I²C address, which allows the ESP32 to identify the device on the bus.

In this setup:

SDA -> GPIO 21
SCL -> GPIO 22

This is the main advantage of I²C. Instead of using many GPIO pins for the LCD, only two communication pins are required.

The ESP32 sends the display data over I²C, and the LCD displays the received characters on its 16x2 screen.
