/*
  Sunscreen UV Passthrough Measurement (Two-Step Mode)
  Sensor: GUVA-S12SD on Pin A3
  Emitter: UV LED on Pin D6
*/

const int UV_LED_PIN = 6;   // Digital Pin 6
const int SENSOR_PIN = A0;  // Analog Pin 3

const int NUM_SAMPLES = 50;  // Average samples to smooth out signal noise
float test1_BaselineVoltage = 0.0;
float test2_SunscreenVoltage = 0.0;

// Read average raw analog value and convert to voltage
float readAverageVoltage() {
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(SENSOR_PIN);
    delay(10);
  }
  float avgRaw = (float)sum / NUM_SAMPLES;
  return (avgRaw * 5.0) / 1023.0; // Convert 10-bit ADC to Volts
}

void printMenu() {
  Serial.println("\n-----------------------------------------");
  Serial.println("  Sunscreen UV Transmission Test Menu   ");
  Serial.println("-----------------------------------------");
  Serial.println("Send '1' -> Take Test 1 (Clear Slide Baseline)");
  Serial.println("Send '2' -> Take Test 2 (Sunscreen Slide Test)");
  Serial.println("-----------------------------------------");
}

void setup() {
  Serial.begin(9600);
  pinMode(UV_LED_PIN, OUTPUT);
  digitalWrite(UV_LED_PIN, HIGH); // Turn on UV LED

  delay(500); // Allow sensor and LED to stabilize
  printMenu();
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    // STEP 1: MEASURE BASELINE (CLEAR SLIDE)
    if (command == '1') {
      Serial.println("\nMeasuring Test 1 (Clear Slide)... Please wait.");
      test1_BaselineVoltage = readAverageVoltage();

      if (test1_BaselineVoltage < 0.05) {
        Serial.println("ERROR: Low UV light detected! Check LED on D6 or sensor on A3.");
      } else {
        Serial.print("Test 1 Baseline Voltage: ");
        Serial.print(test1_BaselineVoltage, 3);
        Serial.println(" V (Set as 100% Reference)");
        Serial.println("\nNow apply sunscreen to your slide, place it back, and send '2'.");
      }
    }

    // STEP 2: MEASURE SUNSCREEN SLIDE
    else if (command == '2') {
      if (test1_BaselineVoltage < 0.05) {
        Serial.println("\nERROR: Please run Test 1 ('1') first to establish a baseline!");
      } else {
        Serial.println("\nMeasuring Test 2 (Sunscreen Slide)... Please wait.");
        test2_SunscreenVoltage = readAverageVoltage();

        float passthroughPercent = (test2_SunscreenVoltage / test1_BaselineVoltage) * 100.0;

        // Clamp upper limit to 100% in case ambient light shifts slightly
        if (passthroughPercent > 100.0) passthroughPercent = 100.0;

        Serial.println("\n========= TEST RESULTS =========");
        Serial.print("Test 1 (Clear Slide) Voltage    : ");
        Serial.print(test1_BaselineVoltage, 3);
        Serial.println(" V");

        Serial.print("Test 2 (Sunscreen Slide) Voltage: ");
        Serial.print(test2_SunscreenVoltage, 3);
        Serial.println(" V");

        Serial.print("UV Transmission Passthrough     : ");
        Serial.print(passthroughPercent, 2);
        Serial.println(" %");

        Serial.print("UV Blocked                      : ");
        Serial.print(100.0 - passthroughPercent, 2);
        Serial.println(" %");
        Serial.println("=================================");

        printMenu();
      }
    }
  }
}