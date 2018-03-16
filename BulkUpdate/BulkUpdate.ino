#include<EthernetClient.h>      // Library to work with ESP-12E board, used to connect to a specified internet IP address and port
#include<ESP8266WiFi.h>         // Library to work with ESP-12E board
#include <SPI.h>                // Library to work with Serial Peripheral Interface (SPI) serial data
#include <Wire.h>               // Library to communicate with I2C devices
#include <Adafruit_MLX90614.h>  // Library to communicate with IR sensors
#include "SSD1306.h"            // Library to work with SSD1306 LOED display
#define SDA 5                   // Serial data line on pin D5
#define SCL 4                   // Serial clock line on pin D4
SSD1306  display(0x3c, SDA, SCL); // Initialise the OLED display using Wire library

Adafruit_MLX90614 IR_1 = Adafruit_MLX90614(0x5A); // Initialise IR sensor address using Adafruit_MLX90614 library
const int LDR = A0;             // Pin for LDR or photodiode

char jsonBuffer[1500] = "[";    // Initialize the jsonBuffer to hold data

char ssid[] = "";               // TODO - insert Network SSID between ""
char pass[] = "";               // TODO - insert Network password between ""
WiFiClient client;              // Initialize WiFi client library

char server[] = "api.thingspeak.com"; // ThingSpeak Server address

/* Collect data once every 1 second and post data to ThingSpeak channel once every 16 seconds */
unsigned long lastConnectionTime = 0;               // Track the last connection time
unsigned long previousUpdate = 0;                   // Track the last update time
const unsigned long postingInterval = 16L * 1000L;  // Post data every 16 seconds
const unsigned long updateInterval = 0.5L * 1000L;  // Update JSON once every 500ms

void setup() {
  display.init();           // Initialize OLED - note must come before Wire.begin(SDA, SCL)
  Wire.begin(SDA, SCL);     // MLX90614 pins for SDA and SCL can now be set
  IR_1.begin();             // Initialize IR sensor
  display.clear();          // Clear OLED Display
  Serial.begin(9600);
  // Attempt to connect to WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network
    delay(10000);           // Wait 10 seconds to connect
    display.clear();        // Clear OLED Display
  }
}

void loop() {
  // If update time has reached 16 seconds, then update the jsonBuffer
  if (millis() - previousUpdate >=  updateInterval) {
    updatesJson(jsonBuffer);
  }
}

// Updates the jsonBuffer with data
void updatesJson(char* jsonBuffer) {
  int ldrVal = analogRead(LDR);
  float voltage = ldrVal * (3.3 / 1023.0);  // Convert ADC input reading to voltage
  // Format the jsonBuffer as noted above
  strcat(jsonBuffer, "{\"delta_t\":"); // Relative timestamp
  unsigned long deltaT = (millis() - previousUpdate) / 1000;
  size_t lengthT = String(deltaT).length(); // Returns length of deltaT string
  char temp[4];
  String(deltaT).toCharArray(temp, lengthT + 1); // Copies deltaT string characters to the temp buffer.
  strcat(jsonBuffer, temp); // Concatenate 'temp' buffer to json buffer
  strcat(jsonBuffer, ",");  // Concatenate ', ' to json buffer
  strcat(jsonBuffer, "\"field1\":");  // Concatenate '\"field1\":' to json buffer
  lengthT = String(IR_1.readObjectTempC()).length(); // Returns length of IR_1.readObjectTempC() string
  String(IR_1.readObjectTempC()).toCharArray(temp, lengthT + 1);  // Data uploaded to ThinkSpeak channel field 1
  strcat(jsonBuffer, temp); // Concatenate 'temp' buffer to json buffer
  strcat(jsonBuffer, ",");  // Concatenate ', ' to json buffer
  strcat(jsonBuffer, "\"field2\":"); // Concatenate '\"field2\":' to json buffer
  String(voltage).toCharArray(temp, lengthT + 1);   // Data uploaded to ThinkSpeak channel field 2
  strcat(jsonBuffer, temp); // Concatenate 'temp' buffer to json buffer
  strcat(jsonBuffer, "},"); // Concatenate '},' buffer to json buffer
  // If posting interval time has reached 16 seconds, update the ThingSpeak channel with data
  if (millis() - lastConnectionTime >=  postingInterval) {
    size_t len = strlen(jsonBuffer);  // Returns length of jsonBuffer
    jsonBuffer[len - 1] = ']';
    httpRequest(jsonBuffer);  // Calls the 'httpRequest' subroutine
  }
  drawOLED();   // Calls the 'drawOLED' subroutine
  previousUpdate = millis(); // Update the last update time
}

// Updates the ThingSpeakchannel with data
void httpRequest(char* jsonBuffer) {
  // Format the data buffer as noted above
  char data[1500] = "{\"write_api_key\":\"CHANNEL-WRITEAPIKEY\",\"updates\":"; // TODO - replace CHANNEL-WRITEAPIKEY with your ThingSpeak channel write API key
  strcat(data, jsonBuffer);
  strcat(data, "}");
  // Close any connection before sending a new request
  client.stop();
  String data_length = String(strlen(data) + 1); // Returns length of 'data' string
  // POST data to ThingSpeak
  if (client.connect(server, 80)) {
    client.println("POST /channels/CHANNEL-ID/bulk_update.json HTTP/1.1"); // TODO - replace CHANNEL-ID with your ThingSpeak channel ID
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: mw.doc.bulk-update (Arduino ESP8266)");
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.println("Content-Length: " + data_length);
    client.println();
    client.println(data);
  }
  jsonBuffer[0] = '[';             // Reinitialize the jsonBuffer for next batch of data
  jsonBuffer[1] = '\0';
  lastConnectionTime = millis();   // Update the last connection time
}

void drawOLED() {
  int ldrVal = analogRead(LDR);
  float voltage = ldrVal * (3.3 / 1023.0);  // Convert ADC input reading to voltage
  display.clear();  // Clear OLED Display
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, String("Temperature: ") + Float2String(IR_1.readObjectTempC()) + String(" C"));
  display.drawString(0, 10, String("Light Intensity: ") + Float2String(voltage) + String(" V"));
  display.display();
}

String Float2String(const float value) {  // Convert a float to string with two decimals
  char temp[12];
  String s;

  dtostrf(value, 8, 2, temp);             // The dtostrf() function converts the double value
  s = String(temp);                       // passed in val into an ASCII representation, stored under s
  s.trim();
  return s;
}
