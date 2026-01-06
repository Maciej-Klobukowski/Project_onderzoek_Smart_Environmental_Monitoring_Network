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

Raspberry Pi 5 Ontvangt data Visualiseert data via Node-RED dashboard

🧩 Hardwarecomponenten
Raspberry Pi 5
<img width="366" height="259" alt="image" src="https://github.com/user-attachments/assets/0a55f759-8324-4f07-b0b9-3abde221550a" />


Wordt gebruikt als centrale ontvanger en dashboardserver.

ESP32 DevKit (AZ-Delivery)
<img width="268" height="231" alt="image" src="https://github.com/user-attachments/assets/fccc729a-6ecc-4d0c-b167-f20e93b091ee" />


18650 Batterijhouder
<img width="335" height="552" alt="image" src="https://github.com/user-attachments/assets/54063d02-81d1-49f2-8d3d-566f5f078721" />

De 18650 batterijhouder wordt gebruikt om een enkele 18650 lithium-ion accu veilig te plaatsen en aan te sluiten.
Deze accu vormt de hoofdstroomvoorziening van het systeem en maakt het project draagbaar en onafhankelijk van netvoeding.


Lithium 18650 Batterijlader met USB-C 5V
<img width="422" height="391" alt="image" src="https://github.com/user-attachments/assets/6839441a-11f6-454e-a6a0-8b306043c8c2" />


Deze laadmodule laadt de 18650 batterij veilig op via USB-C (5V).
De module bevat beveiligingen tegen:

overladen
diepontladen
kortsluiting
Hierdoor kan de batterij betrouwbaar en veilig worden gebruikt in het project.

MT3608 2A Max DC-DC Step Up Power Module Booster Power Module 
<img width="496" height="385" alt="image" src="https://github.com/user-attachments/assets/6293a112-5c56-48a5-a894-93619adabc0d" />

De MT3608 is een DC-DC step-up converter die een lage ingangsspanning (bijv. 3.3–4.2V van de batterij) kan verhogen naar 5V.
Deze wordt gebruikt om:

sensoren met 5V-vereiste (zoals de gassensor) correct te voeden
stabiele spanning te leveren bij wisselende batterijspanning


Verantwoordelijk voor sensoruitlezing en LoRa-transmissie.
LoRa Module – RFM95
Zorgt voor langeafstand draadloze communicatie (868 MHz – EU).

🔌 Sensoren
KY-038 – Geluidsensor
<img width="429" height="350" alt="image" src="https://github.com/user-attachments/assets/17454238-536a-484b-8ae2-105b07e7dd8a" />


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
<img width="579" height="525" alt="image" src="https://github.com/user-attachments/assets/c5ce6d43-a6a0-4563-b2ba-f1f62a43a88e" />


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
<img width="360" height="288" alt="image" src="https://github.com/user-attachments/assets/4832bde4-c909-46c8-9dbd-0ea7d887ca0a" />


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
<img width="398" height="303" alt="image" src="https://github.com/user-attachments/assets/3895584c-6f6f-4931-b5a0-306c2779c52f" />


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

mobiel veilig
geschikt voor buitenopstelling .

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

Indoor monitoring

Educatieve IoT-projecten

Milieuonderzoek

⚠️ Problemen tijdens het project

Tijdens de ontwikkeling van het Smart Environmental Monitoring Network zijn meerdere technische problemen opgetreden. Deze problemen waren leerzaam en hebben geleid tot diepere kennis van hardware-integratie, libraries en datasheets. Hieronder worden de belangrijkste problemen en hun oplossingen beschreven.

🖥️ LCD-aansturing op Raspberry Pi 5
Probleembeschrijving

Tijdens de eerste fase van het project functioneerde de LCD-aansturing op de Raspberry Pi 5 volledig niet.
De gebruikte LCD-library reageerde niet op commando’s, het display werd niet geïnitialiseerd en geen enkele functie leverde een bruikbaar resultaat op.

Aanvankelijk leek het probleem te liggen bij:

de hardware-aansluitingen

I²C-configuratie

of de eigen code

Na uitgebreide tests bleek echter dat geen van deze factoren de oorzaak was.

Oorzaak

Na verdere analyse werd vastgesteld dat het probleem veroorzaakt werd door een bug in de gebruikte LCD-library, specifiek met betrekking tot compatibiliteit met de Raspberry Pi 5.
De library was nog niet aangepast aan de gewijzigde hardware en timing van de Pi 5, waardoor deze in de praktijk onbruikbaar was.

Oplossing

Twee dagen later bracht de ontwikkelaar van de LCD-library een gepatchte update uit.
Na het installeren van deze vernieuwde versie:

werkte de LCD onmiddellijk correct

reageerden alle functies zoals verwacht

kon het project zonder verdere belemmeringen worden voortgezet

Dit probleem benadrukte het belang van:

actuele libraries

compatibiliteitscontrole bij nieuwe hardware

🌤️ Fotosensor KY-018 – Fout en reden voor omgekeerde aansluiting
Beschrijving van de fout

De gebruikte KY-018 fotosensor vertoonde een ontwerpfout op het printplaatje waarbij de signaal- en GND-aansluiting verwisseld zijn. Hierdoor kwam het uitgangssignaal op een verkeerde pin terecht.

Waarom lijkt de aansluiting omgedraaid?

Bij inspectie van de KY-018 valt op dat de layout en de standaard kleurcodering niet overeenkomen met de werkelijke functie van de pinnen. Normaal verwacht men:

VCC op de buitenste pin

GND in het midden

Signaal op de overblijvende pin

Op dit specifieke bordje blijkt echter dat de fabrikant:

het GND-pad en het signaalpad heeft verwisseld

het pad naar de collector van de fototransistor heeft verbonden met de pin die als GND is gemarkeerd

de emitter naar de signaalpin heeft geleid

Dit is tegengesteld aan de gangbare configuratie.

Gevolgen

Wanneer de sensor volgens de standaard pinvolgorde wordt aangesloten:

ontstaat er geen geldig meetbaar uitgangssignaal

kunnen meetwaarden volledig ontbreken

bestaat er risico op kortsluiting of schade aan het bordje

Hierdoor lijkt het alsof de sensor “verkeerd aangesloten” moet worden, terwijl de fout in werkelijkheid in het ontwerp van het sensorbordje zit.

Aanbevolen oplossing

De bedrading moet handmatig worden omgewisseld (signaal ↔ GND)

Het sensorbordje dient te worden gemarkeerd om herhalingsfouten te voorkomen

Alternatief: vervanging door een correct ontworpen KY-018 module

🔥 Gassensor – Correctie op Amazon-informatie en toegepaste oplossing
Probleembeschrijving

De gassensor die via Amazon werd aangeschaft bevatte foutieve spanningsinformatie in de productomschrijving.
Volgens de Amazon-listing zou de sensor correct functioneren op 3.3V.

Na controle van het officiële datasheet bleek echter dat:

de sensor een voedingsspanning van 5V vereist

de interne heater niet correct functioneert op 3.3V

Waarom de Amazon-informatie fout is

Veel low-cost sensoren worden verkocht met:

onvolledige datasheets

vereenvoudigde of foutieve specificaties

Bij gebruik van 3.3V kreeg de interne heater:

onvoldoende vermogen

wat resulteerde in onnauwkeurige, instabiele of volledig ontbrekende metingen

Toegepaste oplossing

Om het probleem correct op te lossen hebben we:

een step-up booster gebruikt om 3.3V om te zetten naar 5V voor de gassensor

de ESP32-spanningsregelaar ingezet om stabiele 3.3V te leveren aan:

overige sensoren

LoRa-module

Resultaat

Na deze aanpassingen:

werkt de gassensor betrouwbaar en stabiel op 5V

blijft de ESP32 veilig op 3.3V functioneren

is het gehele systeem elektrisch gescheiden en stabiel

Dit voorkomt foutmetingen en verhoogt de betrouwbaarheid van het monitoringsysteem aanzienlijk.


