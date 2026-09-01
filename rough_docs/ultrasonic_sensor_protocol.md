Unlike I²C, SPI, or UART where devices exchange structured data bit by bit, the ultrasonic sensor doesn't "talk" in that sense. It uses something much simpler:

TRIG pin = a button you press. The ESP32 briefly presses this "button" to tell the sensor to send out a sound wave.
ECHO pin = a stopwatch signal. The sensor turns this pin ON the instant it sends the sound, and turns it OFF the instant it hears the sound come back. The ESP32 just times how long it stayed ON.
Longer ON time = object is farther away. Shorter ON time = object is closer.

That's it — there's no data packet, no addresses, no clock line. It's just two wires and precise timing, which is why it's often called a trigger-echo or pulse-timing method rather than a "communication protocol" in the traditional sense