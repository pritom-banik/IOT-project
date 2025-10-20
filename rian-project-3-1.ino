#include <ESP32Servo.h>

#define TRIG_PIN 5
#define ECHO_PIN 18
#define MOTOR_PIN 4
#define PROXIMITY_PIN 27
#define SERVO_PIN 32

Servo myServo;

// Calibration and system state variables
float baselineDistance = 2;
bool objectDetected = false;
bool systemBusy = false;
unsigned long motorStopTime = 0;
const int MOTOR_DELAY = 50; // Delay in ms for object to reach proximity sensor

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(PROXIMITY_PIN, INPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(90); // Start at 90° (neutral position)
  
  Serial.println("=== System Started ===");
  Serial.println("Calibrating baseline distance...");
  
 
  
  Serial.println("System ready. Motor running.");
}

void loop() {
  digitalWrite(MOTOR_PIN, HIGH);
  delay(100);
  digitalWrite(MOTOR_PIN, LOW);

  float distance = getDistance();
  
  // If system is busy processing, don't check for new objects
  if (distance >0 && distance <baselineDistance) {
    Serial.println("╔════════════════════════════════╗");
      Serial.print("║ OBJECT DETECTED: ");
      Serial.print(distance);
      Serial.println(" cm      ║");
      Serial.println("║ Motor continuing briefly...    ║");
      Serial.println("╚════════════════════════════════╝");

      digitalWrite(MOTOR_PIN, HIGH); // Stop motor
      delay(100); // Stabilize
      digitalWrite(MOTOR_PIN, LOW); 
      
      // Check proximity sensor for metal detection
      int proximityState = digitalRead(PROXIMITY_PIN);
      
      if (proximityState == LOW) {
        // Metal detected
        Serial.println("🔸 METAL DETECTED after soner sensor- Pushing LEFT (0°)");
        myServo.write(0);
        delay(1000);
        myServo.write(90);
        delay(500);
      } else {
        // Non-metal detected
        Serial.println("🔹 NON-METAL DETECTED - Pushing RIGHT (180°)");
        myServo.write(180);
        delay(1000);
        myServo.write(90);
        delay(500);
      }
    
   // return; // Skip rest of loop while busy
  }
  

  int proximityState = digitalRead(PROXIMITY_PIN);
      
      if (proximityState == LOW) {
        // Metal detected
        Serial.println("🔸 METAL DETECTED without sensor - Pushing LEFT (0°)");
        digitalWrite(MOTOR_PIN, LOW);
        delay(100);        
        myServo.write(0);
        delay(1000);
        myServo.write(90);
        delay(500);
       // digitalWrite(MOTOR_PIN, HIGH);
      }
  
  delay(50); // Loop delay for stability
}

// Function to measure distance in cm
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30 ms timeout
 
  if (duration == 0) return -1; // No echo received
  return (duration * 0.0343) / 2;
}