#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= PINS =================
const int PIN_SOIL   = A0;
const int PIN_SWITCH = A1;
const int PIN_PUMP   = 8;
const int PIN_BUTTON = 7;

// ================= DISPLAY =================
const unsigned long DISPLAY_ON_MS = 5000;
bool displayOn = false;
unsigned long displayOnTs = 0;

// ================= DISPLAY MODUS =================
enum DisplayMode
{
  MODE_GRAPH,
  MODE_TEXT
};

DisplayMode currentMode = MODE_GRAPH;

// ================= PROFILE =================
const int THRESH_WET    = 430;
const int THRESH_NORMAL = 520;
const int THRESH_DRY    = 610;

const unsigned long PUMP_WET_MS    = 1000;
const unsigned long PUMP_NORMAL_MS = 2000;
const unsigned long PUMP_DRY_MS    = 3000;

const unsigned long SAMPLE_INTERVAL_MS = 30000;
const unsigned long DEFAULT_PUMP_TIME = 1000;

const int HYSTERESIS = 20;

// ================= FEUCHTEVERLAUF =================
const int HISTORY_SIZE = 64;
int moistureHistory[HISTORY_SIZE];
int historyIndex = 0;
bool historyFilled = false;

// ================= STATUS =================
unsigned long lastSampleTs = 0;
const int N_SAMPLES = 10;

// =================================================
// ================= FUNKTIONEN ====================
// =================================================

int readSoilAveraged()
{
  long sum = 0;
  for (int i = 0; i < N_SAMPLES; i++)
  {
    sum += analogRead(PIN_SOIL);
    delay(5);
  }
  return sum / N_SAMPLES;
}

int getWaterProfile()
{
  int val = analogRead(PIN_SWITCH);
  if (val < 341) return 1;
  if (val < 682) return 2;
  return 3;
}

void runPump(const unsigned long runTime, const int threshold)
{
  Serial.println(F("*** Pumpvorgang START ***"));

  while (true)
  {
    digitalWrite(PIN_PUMP, HIGH);
    delay(runTime);
    digitalWrite(PIN_PUMP, LOW);

    delay(15000);

    int moisture = readSoilAveraged();
    if (moisture < (threshold + HYSTERESIS))
      break;
  }

  Serial.println(F("*** Pumpvorgang STOP ***"));
}

// ================= DISPLAY CONTROL =================

void displayWake()
{
  if (!displayOn)
  {
    display.ssd1306_command(SSD1306_DISPLAYON);
    displayOn = true;
  }
  displayOnTs = millis();
}

void displaySleepIfTimeout()
{
  if (displayOn && millis() - displayOnTs >= DISPLAY_ON_MS)
  {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    displayOn = false;
  }
}

// ================= FEUCHTEVERLAUF =================

void addMoistureToHistory(int value)
{
  moistureHistory[historyIndex++] = value;
  if (historyIndex >= HISTORY_SIZE)
  {
    historyIndex = 0;
    historyFilled = true;
  }
}

void drawMoistureGraph(int threshold)
{
  if (!displayOn) return;

  const int gx = 0;
  const int gy = 16;
  const int gw = 128;
  const int gh = 48;

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("Feuchteverlauf"));

  display.drawRect(gx, gy, gw, gh, SSD1306_WHITE);

  int count = historyFilled ? HISTORY_SIZE : historyIndex;
  if (count < 2) return;

  for (int i = 1; i < count; i++)
  {
    int idx0 = (historyIndex + i - count - 1 + HISTORY_SIZE) % HISTORY_SIZE;
    int idx1 = (historyIndex + i - count + HISTORY_SIZE) % HISTORY_SIZE;

    int v0 = moistureHistory[idx0];
    int v1 = moistureHistory[idx1];

    int y0 = map(v0, 0, 1023, gy + gh - 2, gy + 1);
    int y1 = map(v1, 0, 1023, gy + gh - 2, gy + 1);

    int x0 = map(i - 1, 0, count - 1, gx + 1, gx + gw - 2);
    int x1 = map(i,     0, count - 1, gx + 1, gx + gw - 2);

    display.drawLine(x0, y0, x1, y1, SSD1306_WHITE);
  }

  int yThresh = map(threshold, 0, 1023, gy + gh - 2, gy + 1);
  display.drawLine(gx + 1, yThresh, gx + gw - 2, yThresh, SSD1306_WHITE);

  display.display();
}

// ================= TEXTANZEIGE =================

void drawTextScreen(int profile, int moisture, int threshold)
{
  if (!displayOn) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print(F("Profil: "));
  display.println(profile);

  display.print(F("Feuchte: "));
  display.println(moisture);

  display.print(F("Schwelle: "));
  display.println(threshold);

  display.display();
}

// ================= SETUP =================

void setup()
{
  pinMode(PIN_PUMP, OUTPUT);
  digitalWrite(PIN_PUMP, LOW);

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println(F("Start: Autobewaesserung mit OLED"));

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("OLED nicht gefunden"));
    while (true);
  }

  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
}

// ================= LOOP =================

void loop()
{
  unsigned long now = millis();

  // ---------- Taster ----------
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(PIN_BUTTON);

  if (lastButtonState == HIGH && buttonState == LOW)
  {
    displayWake();

    // Modus wechseln
    currentMode = (currentMode == MODE_GRAPH) ? MODE_TEXT : MODE_GRAPH;

    delay(200); // Entprellen
  }
  lastButtonState = buttonState;

  displaySleepIfTimeout();

  // ---------- Messintervall ----------
  if (now < lastSampleTs)
    lastSampleTs = now;

  if (now - lastSampleTs >= SAMPLE_INTERVAL_MS)
  {
    lastSampleTs = now;

    int profile = getWaterProfile();
    int moisture = readSoilAveraged();
    addMoistureToHistory(moisture);

    int threshold;
    unsigned long pumpTime;

    switch (profile)
    {
      case 1: threshold = THRESH_WET;    pumpTime = PUMP_WET_MS;    break;
      case 2: threshold = THRESH_NORMAL; pumpTime = PUMP_NORMAL_MS; break;
      case 3: threshold = THRESH_DRY;    pumpTime = PUMP_DRY_MS;    break;
      default: threshold = THRESH_NORMAL; pumpTime = DEFAULT_PUMP_TIME;
    }

    Serial.print(F("Profil: "));
    Serial.print(profile);
    Serial.print(F(" | Feuchte: "));
    Serial.print(moisture);
    Serial.print(F(" | Schwelle: "));
    Serial.println(threshold);

    // ---------- Anzeige ----------
    if (currentMode == MODE_GRAPH)
      drawMoistureGraph(threshold);
    else
      drawTextScreen(profile, moisture, threshold);

    // ---------- Bewässerung ----------
    if (moisture >= (threshold + HYSTERESIS))
      runPump(pumpTime, threshold);
  }
}
