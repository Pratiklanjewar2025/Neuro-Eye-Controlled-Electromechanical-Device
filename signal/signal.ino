#define SAMPLE_RATE 250
#define BAUD_RATE 115200

// EEG and EOG pins
#define EEG_PIN A0         // EEG signal input pin
#define EOG_PIN A1         // EOG signal input pin

// LED pins
#define IN1 9      // White LED for forward
#define IN2 10        // Red LED for stop
#define IN3 11        // LED for left movement
#define IN4 12       // LED for right movement

// Thresholds for EOG
#define THRESHOLD_LEFT 450 // Threshold for left movement
#define THRESHOLD_RIGHT 650 // Threshold for right movement
#define DEBOUNCE_DELAY 2000 // Delay time (in milliseconds) for detecting new movements

unsigned long lastMovementTime = 0; // To track when the last movement was detected
bool movementDetected = false;      // To track if eye movement is being processed

void setup() {
  // Initialize LED pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  forward();
  delay(2000);
  stops();
  // Initialize serial communication
  Serial.begin(BAUD_RATE);
}

void loop() {
  // *EEG Signal Processing (A0): Forward/Stop*
  static unsigned long past = 0;
  unsigned long present = micros();
  unsigned long interval = present - past;
  past = present;

  // Timer for EEG sampling
  static long timer = 0;
  timer -= interval;

  // Sample and send EEG signal data
  if (timer < 0) {
    timer += 1000000 / SAMPLE_RATE;
    int eeg_signal = analogRead(EEG_PIN);
    Serial.println(eeg_signal); // Send EEG data to Python
  }

  // Check for incoming predictions from Python
  if (Serial.available() > 0) {
    String prediction = Serial.readStringUntil('\n');
    prediction.trim(); // Remove trailing newline or spaces

    if (prediction == "1") { // Forward
      forward();
    } else if (prediction == "0") { // Stop
      stops();
    }
  }

  // *EOG Signal Processing (A1): Left/Right Eye Movements*
  int eog_signal = analogRead(EOG_PIN);
  unsigned long currentTime = millis();

  // Check if enough time has passed since the last movement
  if (currentTime - lastMovementTime >= DEBOUNCE_DELAY) {
    if (eog_signal < THRESHOLD_LEFT && !movementDetected) {
      // Left movement
      left();
      delay(600);
      stops();
      movementDetected = true;
      lastMovementTime = currentTime;
    } else if (eog_signal > THRESHOLD_RIGHT && !movementDetected) {
      // Right movement
      right();
      delay(600);
      stops();
      movementDetected = true;
      lastMovementTime = currentTime;
    }
  }

  // Reset conditions if the signal is neutral
  if (eog_signal >= THRESHOLD_LEFT && eog_signal <= THRESHOLD_RIGHT) {
    if (movementDetected) {
      // digitalWrite(LEFT_LED, LOW);
      // digitalWrite(RIGHT_LED, LOW);
      movementDetected = false;
    }
  }
}

void forward(){
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,HIGH);
  digitalWrite(IN4,LOW);
}

void stops(){
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
}

void left(){
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
}

void right(){
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,HIGH);
  digitalWrite(IN4,LOW);
}