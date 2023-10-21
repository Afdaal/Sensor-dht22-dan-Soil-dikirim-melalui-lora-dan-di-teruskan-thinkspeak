#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// Define the pins for the relays
#define relay1Pin 15
#define relay2Pin 16

// Define the pins used by the transceiver module
#define ss 2
#define rst 13
#define dio0 14
#define en 32

// Define the pin for DHT22 sensor
#define dhtPin 4

// Define the pin for soil moisture sensor
#define soilMoisturePin A0

int minValue = 1000;      // Nilai minimum yang telah ditentukan
int maxValue = 4095;      // Nilai maksimum yang telah ditentukan

DHT dht(dhtPin, DHT22);

int counter = 0;

void setup() {
  // Initialize relay pins
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  
  // Initiate the LoRa Enable pin
  pinMode(en, OUTPUT);
  // LoRa chip is Active Low
  digitalWrite(en, LOW);
  
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender");

  // Setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  // Replace the LoRa.begin() argument with your location's frequency 
  // 433E6 for Asia, 866E6 for Europe, 915E6 for North America
  while (!LoRa.begin(915E6)) {
    Serial.println("LoRa Initializing...");
    delay(500);
  }

  // Change sync word to match the receiver
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  // Initialize DHT22 sensor
  dht.begin();
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // Read data from DHT22 sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read data from soil moisture sensor
  int sensorValue = analogRead(soilMoisturePin);

  int soilMoisture = map(sensorValue, minValue, maxValue, 0, 100);

  // Print sensor readings to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Soil Moisture: ");
  Serial.println(soilMoisture);

  // Create a data string
  String sensorData = String(temperature, 1) + "," + String(humidity) + "," + soilMoisture;

  // Send LoRa packet to the receiver
  LoRa.beginPacket();
  LoRa.print(sensorData);
  LoRa.endPacket();

  counter++;

  delay(3000); // Send data every 10 seconds

    // Control Relay 1 based on conditions
  if (temperature < 40 || humidity > 70 || soilMoisture > 14) {
    digitalWrite(relay1Pin, HIGH); // Relay 1 ON
  } else {
    digitalWrite(relay1Pin, LOW); // Relay 1 OFF
  }

  // Control Relay 2 based on conditions
  if (temperature > 60 || humidity < 55 || soilMoisture < 13) {
    digitalWrite(relay2Pin, HIGH); // Relay 2 ON
  } else {
    digitalWrite(relay2Pin, LOW); // Relay 2 OFF
  }
}
