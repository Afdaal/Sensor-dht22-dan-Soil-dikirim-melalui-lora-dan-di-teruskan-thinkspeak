#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Afdal";
const char* password = "12345678";
const String host = "api.thingspeak.com";
const String apiKey = "119RHOOR8JU38AEK"; // Ganti dengan API Key ThingSpeak Anda

LiquidCrystal_I2C lcd(0x27, 16, 2); // Alamat I2C dan ukuran LCD 16x2

void setup() {
// Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  // Initialize the LoRa Enable pin
  pinMode(32, OUTPUT);
  // LoRa chip is Active Low
  digitalWrite(32, LOW);

  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);

  // Setup LoRa transceiver module
  LoRa.setPins(2, 13, 14); // Ganti pin sesuai dengan konfigurasi ESP32
  while (!LoRa.begin(915E6)) {
    Serial.println("LoRa Initializing...");
    delay(500);
  }

  // Change sync word to match the sender
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  // Initialize LCD
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("LoRa Receiver");

  
}

void loop() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    while (LoRa.available()) {
      String receivedData = LoRa.readString();
      Serial.print("Data diterima: ");
      Serial.println(receivedData);

      // Split the received data into temperature, humidity, and soil moisture
      float temperature, humidity;
      int soilMoisture;
      if (sscanf(receivedData.c_str(), "%f,%f,%d", &temperature, &humidity, &soilMoisture) == 3) {
        // Display data on the LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temperat: ");
        lcd.print(temperature);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("Humidity: ");
        lcd.print(humidity);
        lcd.print("%");

        delay(2000); // Display temperature and humidity for 5 seconds

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Moisture: ");
        lcd.print(soilMoisture);
        lcd.print("%");
        lcd.setCursor(0, 1);
        lcd.print("Thanks Afdall");


        // Send data to ThingSpeak
        sendToThingSpeak(temperature, humidity, soilMoisture);
      }
    }
  }
}

void sendToThingSpeak(float temperature, float humidity, int soilMoisture) {
  HTTPClient http;

  String data = "field1=" + String(temperature, 1) + "&field2=" + String(humidity) + "&field3=" + String(soilMoisture);
  String url = "http://" + host + "/update";

  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("X-THINGSPEAKAPIKEY", apiKey);

  int httpResponseCode = http.POST(data);
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.println("Error sending data to ThingSpeak");
  }

  http.end();
}
