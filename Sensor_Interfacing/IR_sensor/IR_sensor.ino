// Define the GPIO pin connected to the IR sensor's OUT pin
#define IR_SENSOR_PIN 15 

void setup() {
  // Start the hardware Serial Monitor at 115200 baud
  Serial.begin(115200); 
  
  // Configure the IR sensor pin as an INPUT
  pinMode(IR_SENSOR_PIN, INPUT); 
}

void loop() {
  // Read the digital state of the sensor (HIGH or LOW)
  int sensorValue = digitalRead(IR_SENSOR_PIN); 
  
  // Most IR modules pull the output LOW when an object reflects the light
  if (sensorValue == LOW) {
    Serial.println("Obstacle Detected!");
  } else {
    Serial.println("Path Clear (No Obstacle)");
  }
  
  delay(200); // Small delay to avoid flooding the Serial Monitor
}
