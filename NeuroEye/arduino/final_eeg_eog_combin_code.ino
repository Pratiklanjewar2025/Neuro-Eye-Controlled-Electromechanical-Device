// Combined sketch: EEG (forward/stop) from code1 + EOG (left/right) from code2

#define SAMPLE_RATE 500
#define BAUD_RATE 230400

// Pins
#define EEG_PIN A0         // EEG signal input (sent to Python)
#define EOG_PIN A1         // EOG signal input (used for left/right detection)

// LED / motor control pins
#define IN1 9      // forward indicator (or forward motor control)
#define IN2 10     // stop indicator
#define IN3 11     // left indicator
#define IN4 12     // right indicator

// EOG thresholds & debounce (from code2)
#define THRESHOLD_LEFT 450   // left eye movement threshold
#define THRESHOLD_RIGHT 650  // right eye movement threshold
#define DEBOUNCE_DELAY 2000  // ms

unsigned long lastMovementTime = 0;
bool movementDetected = false;

// --- EMG/EEG filter class from code1 (IIR example) ---
class EMGFilter {
  private:
    float z1, z2, x1, y_scale;
    int bits_points;

  public:
    EMGFilter() {
      z1 = 0.0;
      z2 = 0.0;
      x1 = 0.0;
      y_scale = 0.0;
      bits_points = 0;
    }

    void set_bits(int bits) {
      bits_points = pow(2, bits);
      if(bits_points == 0) bits_points = 1024;
      y_scale = 2.0 / bits_points;
    }

    float process(int input_value) {
      float output = float(input_value);
      x1 = output - (-0.82523238 * z1) - (0.29463653 * z2);
      output = 0.52996723 * x1 + -1.05993445 * z1 + 0.52996723 * z2;
      z2 = z1;
      z1 = x1;
      return output * y_scale;
    }
};

EMGFilter emgFilter;

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // start in stopped state for safety
  stops();
  delay(500);
  Serial.begin(BAUD_RATE);

  // configure filter (same as code1)
  emgFilter.set_bits(10);
}

void loop() {
  // ----- EEG sampling + send to Python -----
  static unsigned long past = 0;
  unsigned long present = micros();
  unsigned long interval = present - past;
  past = present;

  static long timer = 0;
  timer -= interval;

  if (timer < 0) {
    // schedule next sample
    timer += 1000000L / SAMPLE_RATE;

    // read EEG and filter
    int eeg_raw = analogRead(EEG_PIN);
    float eeg_filtered = emgFilter.process(eeg_raw);

    // send filtered value to Python (5 decimal places)
    Serial.println(eeg_filtered, 5);
  }

  // ----- Receive prediction from Python (forward/stop) -----
  if (Serial.available() > 0) {
    String prediction = Serial.readStringUntil('\n');
    prediction.trim();
    if (prediction == "1") {
      forward();
    } else if (prediction == "0") {
      stops();
    }
  }

  // ----- EOG processing for left/right (from code2) -----
  unsigned long currentMillis = millis();
  int eog_signal = analogRead(EOG_PIN);

  // Only accept new left/right if debounce has passed
  if (currentMillis - lastMovementTime >= DEBOUNCE_DELAY) {
    if (eog_signal < THRESHOLD_LEFT && !movementDetected) {
      // left detected
      left();
      delay(600);        // small action duration (as original)
      stops();
      movementDetected = true;
      lastMovementTime = currentMillis;
    } else if (eog_signal > THRESHOLD_RIGHT && !movementDetected) {
      // right detected
      right();
      delay(600);
      stops();
      movementDetected = true;
      lastMovementTime = currentMillis;
    }
  }

  // Reset movementDetected once signal returns to neutral range
  if (eog_signal >= THRESHOLD_LEFT && eog_signal <= THRESHOLD_RIGHT) {
    movementDetected = false;
  }
}

// --- Movement helper functions ---
void forward() {
  // example: set forward LED/motor on
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  // optional: keep a steering LED to indicate forward (can be kept LOW if undesired)
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void stops() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void left() {
  // left action (as in your sketches)
  digitalWrite(IN1, HIGH);   // keep forward engaged if desired while turning (matches earlier code)
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void right() {
  // right action
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
