/* ------------------------------------------------------
   Ultrasonic Distance Sensor with ESP32
   ------------------------------------------------------
   Connections:
   Ultrasonic VCC  -> ESP32 3V3
   Ultrasonic GND  -> ESP32 GND
   Ultrasonic TRIG -> GPIO 5
   Ultrasonic ECHO -> GPIO 18
   LED (+)         -> GPIO 21
   LED (-)         -> 220 Ohm Resistor -> ESP32 GND
   ------------------------------------------------------ */

const int trigPin = 5;      // Sends the ultrasonic pulse
const int echoPin = 18;     // Receives the reflected pulse
const int ledPin  = 21;     // Warns when an object is close

const int alertDistance = 100;   // Distance (cm) below which LED turns on

void setup() {
  Serial.begin(115200);     // For printing readings to Serial Monitor

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

// Sends a trigger pulse and calculates distance from echo time
long getDistance() {
  // Make sure the trigger pin starts LOW
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a 10 microsecond HIGH pulse to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure how long the echo pin stays HIGH (in microseconds)
  long duration = pulseIn(echoPin, HIGH);

  // Convert travel time into distance using speed of sound
  // Speed of sound = 0.034 cm/us, divide by 2 for round trip
  long distance = duration * 0.034 / 2;

  return distance;
}

void loop() {
  long distanceCm = getDistance();

  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  // Turn the LED on only if a valid object is detected nearby
  bool objectNear = (distanceCm > 0 && distanceCm < alertDistance);
  digitalWrite(ledPin, objectNear ? HIGH : LOW);

  delay(300);   // Small pause before the next reading
}

// Unlike I²C, SPI, or UART where devices exchange structured data bit by bit, the ultrasonic sensor doesn't "talk" in that sense. It uses something much simpler:

// TRIG pin = a button you press. The ESP32 briefly presses this "button" to tell the sensor to send out a sound wave.
// ECHO pin = a stopwatch signal. The sensor turns this pin ON the instant it sends the sound, and turns it OFF the instant it hears the sound come back. The ESP32 just times how long it stayed ON.
// Longer ON time = object is farther away. Shorter ON time = object is closer.

// That's it — there's no data packet, no addresses, no clock line. It's just two wires and precise timing, which is why it's often called a trigger-echo or pulse-timing method rather than a "communication protocol" in the traditional sense


