🌍 Smart Environmental Monitoring Network

ESP32 + LoRa + Raspberry Pi 5

📌 Projectoverzicht

Dit project implementeert een Slim Omgevingsmonitoringsysteem waarbij een ESP32 verschillende omgevingssensoren uitleest en deze data draadloos via LoRa verstuurt naar een Raspberry Pi 5.
De Raspberry Pi ontvangt de data en visualiseert deze met Node-RED.

🧠 Architectuur

ESP32 (in behuizing)

Leest sensoren uit

Verstuurt data via LoRa (RFM95)

LoRa communicatie (868 MHz)

Lange afstand, laag energieverbruik

Raspberry Pi 5

Ontvangt data

Visualiseert data via Node-RED dashboard

🧩 Hardwarecomponenten
Raspberry Pi 5

Wordt gebruikt als centrale ontvanger en dashboardserver.

ESP32 DevKit (AZ-Delivery)

Verantwoordelijk voor sensoruitlezing en LoRa-transmissie.

LoRa Module – RFM95

Zorgt voor langeafstand draadloze communicatie (868 MHz – EU).

🔌 Sensoren
KY-038 – Geluidsensor
Code – Geluidsmeting
const int micPin = 34;  
const int sampleWindow = 50; 
unsigned int sample;

unsigned long startMillis = millis();
unsigned int peakToPeak = 0;
unsigned int signalMax = 0;
unsigned int signalMin = 4095;

while (millis() - startMillis < sampleWindow) {
  sample = analogRead(micPin);
  if (sample < 4095) {
    if (sample > signalMax) signalMax = sample;
    else if (sample < signalMin) signalMin = sample;
  }
}

peakToPeak = signalMax - signalMin;
double voltage = (peakToPeak * 3.3) / 4095.0;
if (voltage <= 0.001) voltage = 0.001;
double dB = 20.0 * log10(voltage / 0.006);

Code-uitleg

Meet piek-tot-piek spanning

Zet ADC-waarde om naar spanning

Gebruikt logaritmische schaal (decibel)

Sensor-uitleg

De KY-038 meet geluidsintensiteit via een microfoon.
De decibelwaarde is een ruwe schatting, geschikt voor trendanalyse.

KY-018 – Lichtsensor
Code – Lichtmeting
int raw = analogRead(KY018_PIN);

if (raw < rawMin) rawMin = raw;
if (raw > rawMax) rawMax = raw;

int span = max(1, rawMax - rawMin);
float pct = (float)(rawMax - raw) * 100.0f / span;

pct *= 1.0f;
if (pct > 100.0f) pct = 100.0f;
if (pct < 0.0f) pct = 0.0f;

Code-uitleg

Dynamische calibratie (min/max)

Omzetting naar 0–100%

Bescherming tegen ruis en foutwaarden

Sensor-uitleg

De KY-018 gebruikt een LDR (lichtafhankelijke weerstand) om omgevingslicht te meten.

AM2301 (DHT21) – Temperatuur & Vochtigheid
Code
#define DHTPIN 4
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);

float hum = dht.readHumidity();
float temp = dht.readTemperature();

Uitleg

Digitale sensor

Betrouwbaarder dan DHT11

Meet luchtvochtigheid (%) en temperatuur (°C)

MQ-5 – Gas-/Rooksensor
Code – Gasmeting
#define MQ2_PIN 33
const float GAS_VOLTAGE_MAX = 3.3;
const int GAS_ADC_RES = 4095;
float GAS_smoothValue = 0;

int GAS_raw = analogRead(MQ2_PIN);
GAS_smoothValue = (GAS_smoothValue * 0.8) + (GAS_raw * 0.2);
float GAS_percent = (GAS_smoothValue / GAS_ADC_RES) * 100.0;

if (GAS_percent > 100.0) GAS_percent = 100.0;
if (GAS_percent < 0.0) GAS_percent = 0.0;

Sensor-uitleg

Detecteert:

LPG

Methaan

Rook / brandbare gassen

📡 LoRa Configuratie & Communicatie
#define SCK 18
#define MISO 19
#define MOSI 23
#define SS 5
#define RST 14
#define DIO0 26

#define LORA_FREQ 868E6

LoRa.beginPacket();
LoRa.printf("Temperature: %.2f °C\n",temp);
LoRa.printf("Humidity: %.2f %%\n",hum);
LoRa.printf("Geluidsniveau: %.2f dB\n",dB);
LoRa.printf("Light Value: %.2f\n",pct);
LoRa.printf("Gas Intensity: %.2f %%\n",GAS_percent);
LoRa.endPacket();

🧱 Behuizing

Alle componenten zijn geïntegreerd in één afgesloten doosje, wat het systeem:

mobiel

veilig

geschikt voor buitenopstelling
maakt.

📊 Raspberry Pi 5 & Node-RED

De Raspberry Pi:

Ontvangt LoRa-pakketten

Decodeert data

Visualiseert waarden met Node-RED dashboards

Voorbeelden:

Temperatuurgrafiek

Gas-alarm

Geluidsniveau-indicatie

Lichtintensiteit

✅ Conclusie

Dit project toont een volledig functioneel IoT-meetsysteem met:

Meerdere sensoren

Betrouwbare langeafstand LoRa-communicatie

Centrale visualisatie op Raspberry Pi 5

Geschikt voor:

Smart cities

Indoor monitoring

Educatieve IoT-projecten

Milieuonderzoek


