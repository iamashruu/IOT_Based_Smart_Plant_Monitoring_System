#define BLYNK_TEMPLATE_ID "give your blynk template id"
#define BLYNK_TEMPLATE_NAME "IOT Plant Monitoring System"
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_NODE_MCU_BOARD

#include <BlynkSimpleEsp8266.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>

#define BLYNK_AUTH_TOKEN "give your blynk auth token"
char auth[] = BLYNK_AUTH_TOKEN;
const char* ssid = "give wifi name";     // Replace with your Wi-Fi SSID
const char* password = "give wifi password";

BlynkTimer timer;
const int sensor_pin = A0;  // Connect Soil moisture analog sensor pin to A0 of NodeMCU
int pump = D6; // pump connection
int relayPin = D5; // Relay control pin
float t, h;

#define DHTPIN 2 //Connect Out pin to D4 in NODE MCU
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

// Define soil moisture thresholds
const int dry_threshold = 30;       // Soil moisture level for dry soil
const int saturated_threshold = 70; // Soil moisture level for saturated soil

void sendSensor()
{
  h = dht.readHumidity();
  t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);
  Serial.print("Temperature : ");
  Serial.print(t);
  Serial.print("    Humidity : ");
  Serial.println(h);
}

void setup() {
  Serial.begin(9600);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("Wi-Fi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  dht.begin();
  
  pinMode(D0, OUTPUT);
  pinMode(relayPin, OUTPUT); // Set relay control pin as OUTPUT
  
  // Initialize the Blynk library
  Blynk.begin(auth, ssid, password, "blynk.cloud", 80);
  
  delay(5000);
  
  pinMode(pump, OUTPUT);
  
  // Set up a timer to call the sendSensor function every 5000 milliseconds (5 seconds)
  timer.setInterval(5000L, sendSensor);
}

void loop() {
  Blynk.run();
  timer.run();

  float soil_moisture;
  soil_moisture = (100 - (analogRead(sensor_pin)/1023.00) * 100.00 )*3.08;

  Blynk.virtualWrite(V1, soil_moisture);
  Serial.print("Soil Moisture = ");
  Serial.print(soil_moisture);
  Serial.println("%");
  Serial.print("Temperature = ");
  Serial.println(t);
  Serial.print("Humidity = ");
  Serial.println(h);
  
  if (soil_moisture < dry_threshold) {
    // Soil is dry, turn on the motor to water the plants
    digitalWrite(pump, HIGH);
    digitalWrite(relayPin, HIGH); // Turn on the relay
    Serial.println("Water Pump On");
    Blynk.virtualWrite(V4, HIGH);
    Blynk.virtualWrite(V5, "Dry");
  } else if (soil_moisture > saturated_threshold) {
    // Soil is saturated, no need to water
    digitalWrite(pump, LOW);
    digitalWrite(relayPin, LOW); // Turn off the relay
    Serial.println("Water Pump Off");
    Blynk.virtualWrite(V4, LOW);
    Blynk.virtualWrite(V5, "Saturated");
  } else {
    // Soil moisture is between dry and saturated thresholds
    // Turn off the pump and relay
    digitalWrite(pump, LOW);
    digitalWrite(relayPin, LOW);
    Serial.println("Water Pump Off");
    Blynk.virtualWrite(V4, LOW);
    Blynk.virtualWrite(V5, "Optimal");
  }
}
