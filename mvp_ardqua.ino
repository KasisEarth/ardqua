// --- Konfiguration ---
const int PIN_SOIL = A0;   // Analog-Pin für Feuchtigkeitssensor
const int PIN_SWITCH = A1; // Schalter
const int PIN_PUMP = 8;    // Digital-Pin für Relais/MOSFET
const int PIN_BUT = 9;     // Button fuer Modus-Umschaltung

// --- Profil-Schwellwerte ---
// Bereich: 0..1023, abhängig von Sensor-Kalibrierung
const int THRESH_WET = 430;    // feuchtigkeitsliebend
const int THRESH_NORMAL = 520; // normal
const int THRESH_DRY = 610;    // trockenheitsliebend

// --- Pumpenlaufzeiten je Profil ---
const unsigned long PUMP_WET_MS = 1000;    // 1s
const unsigned long PUMP_NORMAL_MS = 2000; // 2s
const unsigned long PUMP_DRY_MS = 3000;    // 3s

const int thresholds[3] = {430, 520, 610};
const unsigned long pumpTimes[3] = {1000, 2000, 3000};

// Für geglättete Messung
const int N_SAMPLES = 10;

// TODO: aktuell kurze Intervalle zum Testen
const unsigned long SAMPLE_INTERVAL_MS = 30000; // 30 Sekunden
const unsigned long DEFAULT_PUMP_TIME = 1000;   // nur Fallback

// Optional: Hysterese
const int HYSTERESIS = 20;

// --- Zustandsvariablen ---
unsigned long lastSampleTs = 0;

// Globale Variable, damit sie in Funktionen verändert werden kann
int pumpMode = 0;

void changePumpMode()
{
  if (pumpMode == 2)
  {
    pumpMode = 0;
  }
  else
  {
    pumpMode++;
  }
  Serial.println(F("Pump Modus: "));
  Serial.println(pumpMode);
  delay(1000);
}

int readSoilAveraged()
{
  long sum = 0;
  for (int i = 0; i < N_SAMPLES; ++i)
  {
    sum += analogRead(PIN_SOIL);
    delay(5);
  }
  return (int)(sum / N_SAMPLES);
}

void runPump(const unsigned long runTime, const int threshold)
{
  Serial.println(F("*** Pumpvorgang START ***"));

  while (True)
  {
    digitalWrite(PIN_PUMP, HIGH);
    delay(runTime);
    digitalWrite(PIN_PUMP, LOW);

    delay(15000);

    int moisture = readSoilAveraged();
    if (moisture < (threshold + HYSTERESIS))
    {
      break;
    }
  }

  Serial.println(F("*** Pumpvorgang STOP ***"));
}

void setup()
{
  pinMode(PIN_PUMP, OUTPUT);
  digitalWrite(PIN_PUMP, LOW);
  pinMode(PIN_BUT, INPUT);

  Serial.begin(9600);
  Serial.println(F("Start: Bodenfeuchte-Autobewaesserung + Profilwahlschalter"));
}

void loop()
{
  int threshold = thresholds[0];
  unsigned long pumpTime = pumpTimes[0];

  if (digitalRead(PIN_BUT) == HIGH)
  {
    // Bei Knopfdruck wird der Modus eins hochgestellt
    changePumpMode();
    threshold = thresholds[pumpMode];
    pumpTime = pumpTimes[pumpMode];
  }

  unsigned long now = millis();

  // Wenn millis von vorne beginnt, muss letzter Samplezeitpunkt reseted werden
  if (now < lastSampleTs)
  {
    lastSampleTs = now;
  }

  // 1) Messen im Intervall
  if (now - lastSampleTs >= SAMPLE_INTERVAL_MS)
  {
    lastSampleTs = now;

    int moisture = readSoilAveraged();

    // Debug-Ausgabe
    Serial.print(F("Profil: "));
    Serial.print(pumpMode);
    Serial.print(F(" | Feuchtigkeit: "));
    Serial.print(moisture);
    Serial.print(F(" | Schwelle: "));
    Serial.print(threshold);
    Serial.print(F(" | Modus: "));
    Serial.println(profile);

    // 2) Startet die Pumpe?
    if (moisture >= (threshold + HYSTERESIS))
    {
      runPump(pumpTime, threshold);
    }
  }
}
