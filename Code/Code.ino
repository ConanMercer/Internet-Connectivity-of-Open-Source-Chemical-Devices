const int ldrPin = A0;      // Pin for LDR or photodiode
float ldrReading = 0;       // Analog reading from the sensor divider
float ldrVoltage = 0;       // Variable to store conversion into voltage
                   
const int numReadings = 10; // Array size
int readings[numReadings];  // Readings from the analog input
int readIndex = 0;          // Index of the current reading
int total = 0;              // Moving total
int average = 0;            // Average

void setup() {
  Serial.begin(9600);
  for (int presentReading = 0; presentReading < numReadings; presentReading++) {
    readings[presentReading] = 0;
  }
}
void loop() {
  ldrReading = analogRead(ldrPin);
  ldrVoltage = ldrReading * (3.3 / 1023.0);  // Convert ADC input reading to voltage

  total = total - readings[readIndex];  // Subtract the last reading
  readings[readIndex] = ldrVoltage;     // Read from the sensor
  total = total + readings[readIndex];  // Add the reading to the total
  readIndex = readIndex + 1;            // Advance to the next position in the array
  if (readIndex >= numReadings) {       // if the array is full
    readIndex = 0;                      // continue from the beginning of the array
  }
  average = total / numReadings;        // Calculate the moving average
  Serial.println(ldrReading);           // Print LDR sensor voltage
  Serial.println(average);              // Print LDR sensor running average
  delay(100);                           // 100 millisecond delay between printing
}
