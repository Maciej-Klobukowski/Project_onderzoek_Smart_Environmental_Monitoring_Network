#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "DHT.h"
#include <math.h>

// ============================================================
//                      LoRa (ESP32) PINS
// ============================================================
#define SCK   18
#define MISO  19
#define MOSI  23
#define SS    5
#define RST   14
#define DIO0  26

#define LORA_FREQ 868E6  // EU: 868E6

// ============================================================
//                      SENSOR PINS (AANGEPAST)
// ============================================================
// Temperature/Humidity (DHT21) -> D4 (GPIO4)
#define DHTPIN 4
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);

// Sound sensor (A0) -> D26 (GPIO26)  [ADC1 OK]
static constexpr int micPin = 26;
static constexpr int sampleWindow = 50; // ms
unsigned int sample;

// Light sensor (S) -> D32 (GPIO32)   [ADC1 OK]
#define KY018_PIN 32
uint16_t rawMin = 4095, rawMax = 0;

// Gas sensor (A0) -> D13 (GPIO13)    [ADC2: OK zolang je geen WiFi gebruikt]
#define MQ2_PIN 13
static constexpr int GAS_ADC_RES = 4095;
float GAS_smoothValue = 0.0f;

// ============================================================
//                      HELPERS
// ============================================================
static float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// ============================================================
//                      SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nESP32 LoRa Sender");

  // ---- LoRa init ----
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  LoRa.setTxPower(20);
  LoRa.setSpreadingFactor(12);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("Starting LoRa failed! Check your connections.");
    while (true) { delay(1000); }
  }
  Serial.println("LoRa init succeeded!");

  // ---- Sensors init ----
  dht.begin();

  // ---- ADC setup ----
  analogSetWidth(12);

  // 11db gives ~0-3.3V range (best for typical analog sensors)
  analogSetPinAttenuation(micPin, ADC_11db);
  analogSetPinAttenuation(KY018_PIN, ADC_11db);
  analogSetPinAttenuation(MQ2_PIN, ADC_11db);

  pinMode(micPin, INPUT);
  pinMode(KY018_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);
}

// ============================================================
//                      LOOP
// ============================================================
void loop() {
  // ------------------ SOUND (rough dB estimate) ------------------
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 4095;

  while (millis() - startMillis < (unsigned long)sampleWindow) {
    sample = analogRead(micPin);
    if (sample < 4095) {
      if (sample > signalMax) signalMax = sample;
      if (sample < signalMin) signalMin = sample;
    }
  }

  unsigned int peakToPeak = signalMax - signalMin;
  double voltage = (peakToPeak * 3.3) / 4095.0;
  if (voltage <= 0.001) voltage = 0.001;     // avoid log10(0)
  double dB = 20.0 * log10(voltage / 0.006); // rough calibration

  // ------------------ DHT (Temp/Humidity) ------------------
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  bool dhtOk = !(isnan(hum) || isnan(temp));
  if (!dhtOk) {
    Serial.println("DHT read failed!");
  }

  // ------------------ LIGHT (0..100%) ------------------
  int raw = analogRead(KY018_PIN);

  if (raw < rawMin) rawMin = raw;
  if (raw > rawMax) rawMax = raw;

  int span = max(1, (int)rawMax - (int)rawMin);

  // Inverted mapping (common with LDR modules: more light => lower ADC)
  float lightPct = (float)(rawMax - raw) * 100.0f / (float)span;
  lightPct = clampf(lightPct, 0.0f, 100.0f);

  // ------------------ GAS (0..100%) ------------------
  int GAS_raw = analogRead(MQ2_PIN);
  GAS_smoothValue = (GAS_smoothValue * 0.8f) + ((float)GAS_raw * 0.2f);

  float gasPct = (GAS_smoothValue / (float)GAS_ADC_RES) * 100.0f;
  gasPct = clampf(gasPct, 0.0f, 100.0f);

  // ------------------ PRINT ------------------
  Serial.println("Sending packet:");
  if (dhtOk) {
    Serial.printf("  Temperature: %.2f C\n", temp);
    Serial.printf("  Humidity: %.2f %%\n", hum);
  } else {
    Serial.println("  Temperature: NaN");
    Serial.println("  Humidity: NaN");
  }
  Serial.printf("  Sound: %.2f dB\n", dB);
  Serial.printf("  Light: %.2f %%\n", lightPct);
  Serial.printf("  Gas: %.2f %%\n", gasPct);

  // ------------------ SEND LoRa ------------------
  LoRa.beginPacket();

  if (dhtOk) {
    LoRa.printf("maciej_Temperature: %.2f C\n", temp);
    LoRa.printf("maciej_Humidity: %.2f %%\n", hum);
  } else {
    LoRa.print("maciej_Temperature: NaN\n");
    LoRa.print("maciej_Humidity: NaN\n");
  }

  LoRa.printf("maciej_Sound: %.2f dB\n", dB);
  LoRa.printf("maciej_Light: %.2f %%\n", lightPct);
  LoRa.printf("maciej_Gas: %.2f %%\n", gasPct);

  LoRa.endPacket();

  delay(1000);
}
