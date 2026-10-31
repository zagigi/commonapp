// ----- Pin Definitions -----
#define SENSOR_PIN A0   // Soil moisture sensor pin
#define PUMP_PIN 12      // Relay module pin controlling the pump

void setup() {
  Serial.begin(9600);

  pinMode(SENSOR_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  
  // Keep pump off at system startup
  digitalWrite(PUMP_PIN, LOW); 

  Serial.println("Nano Smart Watering System Initialized");
}

void loop() {
  // ----- Read Soil Moisture -----
  int sensor_data = analogRead(SENSOR_PIN);
  String moisture_level;

  // ----- Pump Control Logic -----
  if (sensor_data > 1000) {
    moisture_level = "Dry";
    digitalWrite(PUMP_PIN, HIGH);  // Turn on pump relay
  }
  else if (sensor_data >= 400 && sensor_data <= 1000) {
    moisture_level = "Medium";
    digitalWrite(PUMP_PIN, LOW);   // Turn off pump relay
  }
  else {
    moisture_level = "Wet";
    digitalWrite(PUMP_PIN, LOW);   // Turn off pump relay
  }

  // ----- Serial Output -----
  Serial.print("Soil Sensor Raw: ");
  Serial.print(sensor_data);
  Serial.print("\t| Condition: ");
  Serial.println(moisture_level);

  // 1-second delay before checking again (reduces jitter/relay wear)
  delay(1000); 
}