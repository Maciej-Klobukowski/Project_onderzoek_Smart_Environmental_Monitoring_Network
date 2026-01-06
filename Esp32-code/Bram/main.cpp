/*
CONNECTIONS:

to lora:
SCK       -->    GPIO18
MISO      -->    GPIO19
MOSI      -->    GPIO23
NSS(CS)   -->    GPIO5
RESET     -->    GPIO14
DIO0      -->    GPIO26

to sensors:
TEMPERATURE (YELLOW)    -->    GPIO4
SOUND (A0)              -->    GPIO34
LightSensor(S)          -->    GPIO32
GasSensor(A0)           -->    GPIO33
*/



#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "DHT.h" //for temp sensor


// --- Pin definitions for ESP32 DevKit (AZ Delivery) ---
#define SCK     18  // SPI Clock
#define MISO    19  // SPI MISO
#define MOSI    23  // SPI MOSI
#define SS      5   // Chip Select (NSS)
#define RST     14  // Reset
#define DIO0    26  // IRQ (RX Done)

//For Temperature and Humidity Sensor
#define DHTPIN 4        // GPIO 4 for DATA (yellow wire)
#define DHTTYPE DHT21   // AM2301A uses the DHT21 protocol
DHT dht(DHTPIN, DHTTYPE);

//For Decibel Measurement
const int micPin = 34;  // analoge pin (A0 van KY-038)
const int sampleWindow = 50; // tijdsvenster in ms voor meting
unsigned int sample;

//For Light Sensor
#define KY018_PIN 32 
uint16_t rawMin = 4095, rawMax = 0;

//For Gas Sensor
#define MQ2_PIN 33   // Analog input pin (after voltage divider)
const float GAS_VOLTAGE_MAX = 3.3;  // ESP32 ADC specs
const int GAS_ADC_RES = 4095;
float GAS_smoothValue = 0;    // Smoothing filter (better readings)

// --- LoRa frequency (set to match your region) ---
#define LORA_FREQ 868E6   // use 868E6 for EU, 433E6 for Asia

void setup() { //For LoRa communication!
  Serial.begin(115200);
  while (!Serial);

  Serial.println("ESP32 LoRa Sender");
  LoRa.setTxPower(20);
  LoRa.setSpreadingFactor(12);
  // Setup LoRa transceiver module
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("Starting LoRa failed! Check your connections.");
    while (true);
  }

  Serial.println("LoRa init succeeded!");

  dht.begin(); //Tempsensor
  
//FOR LIGHTSENSOR  
  analogSetWidth(12);
  analogSetPinAttenuation(KY018_PIN, ADC_11db);
  pinMode(KY018_PIN, INPUT);

}

void loop() {
//Code for Decibel Measurement:
/////////////////////////////////////////////////////////////////////////////////////////////
  unsigned long startMillis = millis();
  unsigned int peakToPeak = 0;
  unsigned int signalMax = 0;
  unsigned int signalMin = 4095;

  // meet het signaal gedurende sampleWindow
  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(micPin);
    if (sample < 4095) { // geldig signaal
      if (sample > signalMax) signalMax = sample;
      else if (sample < signalMin) signalMin = sample;
    }
  }

  peakToPeak = signalMax - signalMin;  // verschil tussen max en min
  double voltage = (peakToPeak * 3.3) / 4095.0; // omzetten naar spanning
  if (voltage <= 0.001) voltage = 0.001;  // voorkom log10(0)
  double dB = 20.0 * log10(voltage / 0.006);    // schatting in decibel (ruw)


//Floats for temp sensor:
/////////////////////////////////////////////////////////////////////////////////////////////
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();


//Code for Light Sensor:
/////////////////////////////////////////////////////////////////////////////////////////////
  int raw = analogRead(KY018_PIN);

  // Leer dynamisch min/max
  if (raw < rawMin) rawMin = raw;
  if (raw > rawMax) rawMax = raw;

  // Bescherm tegen deling door 0
  int span = max(1, rawMax - rawMin);

  // Map naar 0..100 (kies omgekeerd of niet)
  // Als bij jou meer licht -> lagere raw: gebruik inverted mapping:
  float pct = (float)(rawMax - raw) * 100.0f / span;

  // (Als jouw bedrading ooit wordt omgedraaid, gebruik dan: (raw - rawMin) i.p.v. (rawMax - raw))

  // Optionele gain + clamp
  pct *= 1.0f; // pas aan naar smaak
  if (pct > 100.0f) pct = 100.0f;
  if (pct < 0.0f)   pct = 0.0f;


//Code for GAS sensor:
/////////////////////////////////////////////////////////////////////////////////////////////
int GAS_raw = analogRead(MQ2_PIN);    // Read raw ADC
GAS_smoothValue = (GAS_smoothValue * 0.8) + (GAS_raw * 0.2);      // Simple low-pass filter (smooths noise)
float GAS_percent = (GAS_smoothValue / GAS_ADC_RES) * 100.0;    // Convert to percentage 0–100%

  if (GAS_percent > 100.0) GAS_percent = 100.0;   // Safety clamp
  if (GAS_percent < 0.0)   GAS_percent = 0.0;

//Code for all sensors / LoRa communication:
/////////////////////////////////////////////////////////////////////////////////////////////
  Serial.printf("Sending packet: Temperature: %.2f °C\n",temp);
  Serial.printf("Sending packet: Humidity: %.2f %%\n",hum);
  Serial.printf("Sending packet: Geluidsniveau: %.2f dB\n",dB);
  Serial.printf("Sending packet: Light Value: %.2f\n",pct); 
  Serial.printf("Sending packet: Gas Intensity: %.2f %%\n",GAS_percent);


  LoRa.beginPacket();
  LoRa.printf("    \n");
  LoRa.printf("    Temperature: %.2f °C\n",temp);
  LoRa.printf("    Humidity: %.2f %%\n",hum);
  LoRa.printf("    Geluidsniveau: %.2f dB\n",dB);
  LoRa.printf("    Light Value: %.2f\n",pct); 
  LoRa.printf("    Gas Intensity: %.2f %%\n",GAS_percent);
  LoRa.endPacket();

  delay(1000); // Send every second
}