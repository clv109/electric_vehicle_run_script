// Motor and button pins
const int ena = 5;
const int in1 = 6; 
const int in2 = 7;
const int buttonPin = 10; 

// Encoder pins
const int encoderPinA = 2;
const int encoderPinB = 3;

// FORWARD distance settings
const unsigned long forwardFastTimeMs = 10000; // Time spent at speed 225 in ms
const unsigned long forwardSlowTimeMs = 2080; // Time spent at decelerated speed 175 in ms

// --- REVERSE settings
const long reverseTargetTicks = 1890;  //time spent at speed 225 in reverse (encoder ticks not ms)

volatile long encoderCount = 0;

void setup() {
  pinMode(ena, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(ena, 0);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);

  Serial.begin(9600);
  Serial.println("Waiting for button press to start...");

  while (digitalRead(buttonPin) == HIGH) { //pullup resistor is HIGH by default, click will turn it to LOW moving the code forward
    delay(50); 
  }

  Serial.println("Button pressed! Starting in 1 second...");
  delay(1000); 
}

void loop() {
  // ==========================================
  // PHASE 1: FORWARD MOVEMENT (Time-Based)
  // ==========================================
  Serial.println("--- FORWARD PHASE ---");
  digitalWrite(in1, HIGH); 
  digitalWrite(in2, LOW);
  
  // Forward Fast
  analogWrite(ena, 225);
  delay(forwardFastTimeMs); // Time is defined above 

  // Forward Decelerate
  analogWrite(ena, 110); 
  delay(forwardSlowTimeMs); // Time is defined above

  // Power Off
  analogWrite(ena, 0); 
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  
  Serial.println("Forward phase complete (Time-based).");

  delay(2000);  //Small pause, let the vehicle coast to a stop (physical wingnut is used for braking)

  // ==========================================
  // PHASE 2: REVERSE MOVEMENT (Encoder-Based)
  // ==========================================
  Serial.println("--- REVERSE PHASE ---");
  
  noInterrupts(); 
  encoderCount = 0; 
  interrupts();
  
  digitalWrite(in1, LOW); 
  digitalWrite(in2, HIGH); //Flip motor polarity since going backward
  analogWrite(ena, 255);
  
  bool targetReached = false;
  while (!targetReached) {
    long safeCount;
    
    noInterrupts();
    safeCount = encoderCount; // Grab a clean copy of the count
    interrupts();             // Immediately turn interrupts back on
    
    if (abs(safeCount) >= reverseTargetTicks) {
      targetReached = true;
    }
  }

  // Reverse Dynamic Brake (regenerative braking, put both motors on HIGH)
  digitalWrite(in1, HIGH);
  digitalWrite(in2, HIGH);
  analogWrite(ena, 255);
  delay(500);
  
  // Reverse Active Forward Brake
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(ena, 255);
  delay(60);

  // Power Off
  analogWrite(ena, 0); 
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  
  Serial.println("Run complete.");
  
  delay(300000); 
}

void updateEncoder() {
  if (digitalRead(encoderPinA) == digitalRead(encoderPinB)) {
    encoderCount++; 
  } else {
    encoderCount--; 
  }
}
