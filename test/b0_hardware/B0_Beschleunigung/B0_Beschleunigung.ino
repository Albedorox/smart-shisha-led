// B0 – Beschleunigungstest LIS2DW12 (Wegwerfcode, nicht Teil der Firmware)
//
// Prüft, ob der LIS2DW12 das Blubbern beim Ziehen durch Glas und Deckplatte
// erkennt (R-02, Bewertung: docs/planung/12_vibrationssensor_bewertung.md §7).
//
// Ablauf:
//   1. Chip-Kennung (WHO_AM_I) prüfen: 0x44 = LIS2DW12.
//   2. Sensor mit 400 Hz, High-Performance, Low-Noise, ±2 g über den FIFO lesen.
//   3. Bandpass 20 … 150 Hz, Effektivwert je 250-ms-Fenster ("Pegel" in mg).
//   4. 10 s Ruhe messen → Basis. Pegel (gleitend über 0,5 s) über Schwelle
//      (Faktor × Basis) = Ereignis; ab 1 s Dauer ein Zug, sonst ein Stoß.
//   5. Der LED-Ring zeigt den Pegel als Balken (8 LEDs je Verdopplung).
//   6. Optional: SW-420 an GPIO0 läuft mit, seine Flanken werden mitgezählt.
//
// Bedienung über den seriellen Monitor (115200 Baud, Zeilenende egal):
//   0 Ruhe   1 Zug   2 Schlauch anfassen   3 Tisch antippen   x kein Abschnitt
//   e Auswertung der Abschnitte    f Spektrum der letzten 1,28 s
//   k Ruhe neu messen   a Z-Achse / alle Achsen   + - Schwelle
//   p Plotter-Ausgabe   r Rohdaten 400 Hz   q nur Ereignisse   h Hilfe

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// ---------- Anschlüsse (02_hardware §2) ----------
constexpr uint8_t  PIN_SDA    = 6;       // LIS2DW12 SDA
constexpr uint8_t  PIN_SCL    = 7;       // LIS2DW12 SCL
constexpr uint8_t  PIN_LED    = 4;       // LED-Ring DI (über Pegelwandler)
constexpr uint8_t  PIN_SW420  = 0;       // SW-420 DO, optional zum Vergleich
constexpr bool     USE_SW420  = true;    // false, wenn kein SW-420 angeschlossen ist
constexpr uint16_t LED_COUNT  = 24;
constexpr uint8_t  LED_BRIGHT = 40;      // niedrig: Test, kein Licht-Effekt
constexpr uint32_t I2C_HZ     = 400000;

// ---------- LIS2DW12 (ST AN5038) ----------
constexpr uint8_t REG_WHO_AM_I     = 0x0F;
constexpr uint8_t REG_CTRL1        = 0x20;   // ODR[7:4] MODE[3:2] LP_MODE[1:0]
constexpr uint8_t REG_CTRL2        = 0x21;   // BOOT SOFT_RESET - CS_PU_DISC BDU IF_ADD_INC I2C_DISABLE SIM
constexpr uint8_t REG_CTRL6        = 0x25;   // BW_FILT[7:6] FS[5:4] FDS LOW_NOISE - -
constexpr uint8_t REG_OUT_X_L      = 0x28;
constexpr uint8_t REG_FIFO_CTRL    = 0x2E;   // FMode[7:5] FTH[4:0]
constexpr uint8_t REG_FIFO_SAMPLES = 0x2F;   // FIFO_FTH FIFO_OVR DIFF[5:0]

constexpr uint8_t ID_LIS2DW12 = 0x44;
constexpr uint8_t ID_LIS2DS12 = 0x43;
constexpr uint8_t ID_LIS3DH   = 0x33;   // auch LIS2DH12

constexpr uint8_t CTRL1_400HZ_HP   = 0x74;  // ODR 0111 = 400 Hz, MODE 01 = High-Performance
constexpr uint8_t CTRL2_BDU_INC    = 0x0C;  // BDU + Adress-Autoinkrement
constexpr uint8_t CTRL6_2G_LOWNOISE = 0x04; // Filter ODR/2, ±2 g, LOW_NOISE
constexpr uint8_t FIFO_CONTINUOUS  = 0xC0;  // FMode 110
constexpr float   MG_PER_LSB       = 0.244f; // 14 bit, ±2 g (nach >> 2)
constexpr float   NOISE_UG_RTHZ    = 90.0f;  // Datenblatt: HP + LOW_NOISE, ±2 g

// ---------- Auswertung ----------
constexpr float    FS_HZ        = 400.0f;
constexpr float    BAND_LO_HZ   = 20.0f;
constexpr float    BAND_HI_HZ   = 150.0f;
constexpr uint16_t WIN_SAMPLES  = 100;          // 250 ms
constexpr float    WIN_S        = WIN_SAMPLES / FS_HZ;
constexpr uint16_t SETTLE_SAMPLES = 400;        // 1 s Einschwingen der Filter
constexpr uint16_t CALIB_WINDOWS  = 40;         // 10 s Ruhe
constexpr uint8_t  END_WINDOWS    = 2;          // 0,5 s unter Haltegrenze = Ende
constexpr float    HOLD_RATIO     = 0.75f;      // Haltegrenze = 75 % der Schwelle (Hysterese)
constexpr uint8_t  MIN_PULL_WINDOWS = 4;        // ab 1 s Zug, darunter Stoß
constexpr uint16_t MAX_EVENT_WINDOWS = 240;     // 60 s
constexpr uint8_t  SPECTRUM_AT_WINDOW = 5;      // Spektrum nach 1,25 s im Zug
constexpr float    LEDS_PER_DOUBLING = 8.0f;
constexpr float    FACTORS[] = {1.5f, 2.0f, 2.5f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f};
constexpr uint8_t  N_FACTORS = sizeof(FACTORS) / sizeof(FACTORS[0]);
constexpr int      FFT_N = 512;                 // 1,28 s, 0,78 Hz Auflösung

// ---------- Filter ----------
struct Biquad {
  float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
  float step(float x) {
    const float y = b0 * x + z1;
    z1 = b1 * x - a1 * y + z2;
    z2 = b2 * x - a2 * y;
    return y;
  }
  // |H|^2 bei Frequenz f
  float gain2(float f) const {
    const float w = 2.0f * (float)M_PI * f / FS_HZ;
    const float c1 = cosf(w), s1 = sinf(w), c2 = cosf(2 * w), s2 = sinf(2 * w);
    const float nr = b0 + b1 * c1 + b2 * c2, ni = -(b1 * s1 + b2 * s2);
    const float dr = 1 + a1 * c1 + a2 * c2, di = -(a1 * s1 + a2 * s2);
    return (nr * nr + ni * ni) / (dr * dr + di * di);
  }
};

// Butterworth 2. Ordnung (Audio-EQ-Cookbook, Q = 0,7071)
Biquad makeBiquad(float f0, bool highpass) {
  const float w0 = 2.0f * (float)M_PI * f0 / FS_HZ;
  const float c = cosf(w0), alpha = sinf(w0) / (2.0f * 0.70710678f);
  const float a0 = 1 + alpha;
  Biquad q;
  if (highpass) {
    q.b0 = (1 + c) / 2 / a0; q.b1 = -(1 + c) / a0; q.b2 = (1 + c) / 2 / a0;
  } else {
    q.b0 = (1 - c) / 2 / a0; q.b1 = (1 - c) / a0;  q.b2 = (1 - c) / 2 / a0;
  }
  q.a1 = -2 * c / a0;
  q.a2 = (1 - alpha) / a0;
  return q;
}

struct BandFilter {
  Biquad hp, lp;
  void init() { hp = makeBiquad(BAND_LO_HZ, true); lp = makeBiquad(BAND_HI_HZ, false); }
  float step(float x) { return lp.step(hp.step(x)); }
};

// ---------- Zustand ----------
Adafruit_NeoPixel ring(LED_COUNT, PIN_LED, NEO_GRB + NEO_KHZ800);

uint8_t  sensorAddr = 0;
bool     sensorOk   = false;
BandFilter fx, fy, fz;

volatile uint32_t swEdges = 0;
uint32_t swEdgesSeen = 0;

// Fenster
struct Window {
  uint16_t n = 0;
  float sx = 0, sy = 0, sz = 0;    // Quadratsummen gefiltert
  float peakZ = 0, peakAll = 0;    // Spitzenwert gefiltert
  float gx = 0, gy = 0, gz = 0;    // Mittelwert roh (Schwerkraft)
} win;
uint32_t samplesSeen = 0;
uint32_t fifoOverruns = 0, i2cErrors = 0;

float    prevMsZ = 0, prevMsAll = 0;  // mittleres Quadrat des Vorfensters

// Kalibrierung
bool     calibrating = true;
uint16_t calibCount = 0;
float    calibZ[CALIB_WINDOWS], calibAll[CALIB_WINDOWS];
float    baseZ = 1, baseAll = 1;
float    gravX = 0, gravY = 0, gravZ = 0;
float    expectedZ = 0, expectedAll = 0;

// Einstellungen
bool    useAll = false;           // false: nur Z-Achse
uint8_t factorIdx = 1;            // 2,0
enum class Out { Normal, Plot, Raw, Quiet };
Out     outMode = Out::Normal;
int8_t  label = -1;               // -1 kein Abschnitt
const char* const LABELS[] = {"Ruhe", "Zug", "Schlauch", "Tisch"};

// Statistik je Abschnitt (beide Metriken, damit 'a' nichts verfälscht)
struct Stat {
  uint32_t n = 0;
  double sumZ = 0, sumAll = 0;
  float maxZ = 0, maxAll = 0;
  uint32_t overZ = 0, overAll = 0;
  uint32_t sw = 0;
} stats[4];

// Ereignis
bool     evActive = false;
uint16_t evWins = 0, evBelow = 0;
float    evMaxLevel = 0, evMaxPeak = 0, evDomHz = 0;
bool     evSpecDone = false;
uint32_t pullCount = 0, shockCount = 0;

// Sekundenzeile
uint8_t  secWins = 0;
float    secMaxLevel = 0, secMaxPeak = 0;
uint32_t secSw = 0;
bool     secAbove = false;

// Spektrum
float zBuf[FFT_N];
int   zPos = 0;
float fre[FFT_N], fim[FFT_N];

// ---------- I²C ----------
bool writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(sensorAddr);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool readRegs(uint8_t reg, uint8_t* buf, uint8_t len) {
  Wire.beginTransmission(sensorAddr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(sensorAddr, len) != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

bool readReg(uint8_t reg, uint8_t& val) { return readRegs(reg, &val, 1); }

bool probe(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

void IRAM_ATTR onSw420() { swEdges++; }

// ---------- LED-Ring ----------
void ringFill(uint32_t color) {
  for (uint16_t i = 0; i < LED_COUNT; i++) ring.setPixelColor(i, color);
  ring.show();
}

void ringError(uint32_t color) {   // blinkt endlos
  for (;;) {
    ringFill(color); delay(300);
    ringFill(0);     delay(300);
  }
}

int ledsForRatio(float r) {
  if (r <= 1.0f) return 0;
  const int n = (int)lroundf(log2f(r) * LEDS_PER_DOUBLING);
  return n > (int)LED_COUNT ? LED_COUNT : n;
}

void ringShowLevel(float ratio, bool above) {
  const int n = ledsForRatio(ratio);
  const int mark = ledsForRatio(FACTORS[factorIdx]) - 1;   // Schwellenmarke
  for (int i = 0; i < (int)LED_COUNT; i++) {
    uint32_t c = 0;
    if (i < n) c = above ? ring.Color(0, 80, 255) : ring.Color(0, 160, 30);
    else if (i == mark) c = ring.Color(60, 0, 0);
    ring.setPixelColor(i, c);
  }
  ring.show();
}

void ringShowCalib() {
  ring.clear();
  ring.setPixelColor(calibCount % LED_COUNT, ring.Color(80, 80, 80));
  ring.show();
}

// ---------- Sensor ----------
bool initSensor() {
  Wire.begin((int)PIN_SDA, (int)PIN_SCL, I2C_HZ);

  Serial.print("I2C-Scan (SDA GPIO6, SCL GPIO7):");
  uint8_t found = 0;
  for (uint8_t a = 1; a < 127; a++) {
    if (probe(a)) { Serial.printf(" 0x%02X", a); found++; }
  }
  Serial.println(found ? "" : " nichts gefunden");

  const uint8_t candidates[] = {0x19, 0x18};   // SA0 offen (interner Pull-up) = 0x19
  bool identified = false;
  for (uint8_t a : candidates) {
    if (!probe(a)) continue;
    sensorAddr = a;
    uint8_t id = 0;
    if (!readReg(REG_WHO_AM_I, id)) continue;
    Serial.printf("Adresse 0x%02X, WHO_AM_I = 0x%02X -> ", a, id);
    if (id == ID_LIS2DW12) {
      Serial.println("LIS2DW12 erkannt. Passt.");
      identified = true;
      break;
    }
    if (id == ID_LIS2DS12) Serial.println("LIS2DS12! Das ist der Vorgänger, nicht der bestellte Chip. Bitte zurückschicken.");
    else if (id == ID_LIS3DH) Serial.println("LIS3DH/LIS2DH12, nicht der bestellte Chip.");
    else Serial.println("unbekannter Chip.");
    ringError(ring.Color(120, 50, 0));   // orange: falscher Chip
  }
  if (!identified) {
    Serial.println("FEHLER: Kein Sensor an 0x18/0x19. Verdrahtung prüfen: VCC an 3V3, GND, SDA an GPIO6, SCL an GPIO7.");
    ringError(ring.Color(120, 0, 0));    // rot: kein Sensor
  }

  // Reset laut AN5038 §5.9: SOFT_RESET, dann BOOT, 20 ms warten
  bool ok = writeReg(REG_CTRL2, 0x40);
  delay(1);
  ok &= writeReg(REG_CTRL2, 0x80);
  delay(25);
  ok &= writeReg(REG_CTRL2, CTRL2_BDU_INC);
  ok &= writeReg(REG_CTRL6, CTRL6_2G_LOWNOISE);
  ok &= writeReg(REG_FIFO_CTRL, 0x00);           // Bypass leert den FIFO
  ok &= writeReg(REG_FIFO_CTRL, FIFO_CONTINUOUS);
  ok &= writeReg(REG_CTRL1, CTRL1_400HZ_HP);
  uint8_t c1 = 0, c6 = 0;
  ok &= readReg(REG_CTRL1, c1) && readReg(REG_CTRL6, c6);
  if (!ok || c1 != CTRL1_400HZ_HP || c6 != CTRL6_2G_LOWNOISE) {
    Serial.printf("FEHLER: Konfiguration nicht übernommen (CTRL1 0x%02X, CTRL6 0x%02X).\n", c1, c6);
    ringError(ring.Color(120, 0, 0));
  }
  Serial.println("Sensor: 400 Hz, High-Performance, Low-Noise, ±2 g, FIFO kontinuierlich.");
  return true;
}

// ---------- Spektrum ----------
void fft(float* re, float* im, int n) {
  for (int i = 1, j = 0; i < n; i++) {           // Bit-Umkehr
    int bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) { float t = re[i]; re[i] = re[j]; re[j] = t; t = im[i]; im[i] = im[j]; im[j] = t; }
  }
  for (int len = 2; len <= n; len <<= 1) {
    const float ang = -2.0f * (float)M_PI / len;
    const float wr = cosf(ang), wi = sinf(ang);
    for (int i = 0; i < n; i += len) {
      float cr = 1, ci = 0;
      for (int k = 0; k < len / 2; k++) {
        const int a = i + k, b = a + len / 2;
        const float vr = re[b] * cr - im[b] * ci, vi = re[b] * ci + im[b] * cr;
        re[b] = re[a] - vr; im[b] = im[a] - vi;
        re[a] += vr;        im[a] += vi;
        const float t = cr * wr - ci * wi;
        ci = cr * wi + ci * wr; cr = t;
      }
    }
  }
}

struct Spectrum {
  float peakHz[3] = {0, 0, 0}, peakMg[3] = {0, 0, 0};   // Amplitude (Spitze) in mg
  float band[5] = {0, 0, 0, 0, 0};                       // Effektivwert in mg
};
const float BAND_EDGES[6] = {5, 20, 50, 100, 150, 200};

// Z-Achse der letzten 1,28 s (roh, Mittelwert abgezogen, Hann-Fenster)
Spectrum computeSpectrum() {
  float mean = 0;
  for (int i = 0; i < FFT_N; i++) mean += zBuf[i];
  mean /= FFT_N;
  float sw = 0, sw2 = 0;
  for (int i = 0; i < FFT_N; i++) {
    const float w = 0.5f * (1 - cosf(2 * (float)M_PI * i / (FFT_N - 1)));
    fre[i] = (zBuf[(zPos + i) % FFT_N] - mean) * w;
    fim[i] = 0;
    sw += w; sw2 += w * w;
  }
  fft(fre, fim, FFT_N);
  const float df = FS_HZ / FFT_N;
  Spectrum s;
  double bandSum[5] = {0, 0, 0, 0, 0};
  for (int k = 1; k < FFT_N / 2; k++) {
    const float p = fre[k] * fre[k] + fim[k] * fim[k];
    const float f = k * df;
    for (int b = 0; b < 5; b++)
      if (f >= BAND_EDGES[b] && f < BAND_EDGES[b + 1]) bandSum[b] += p;
    // lokale Maxima zwischen 5 und 200 Hz, die drei größten merken
    if (f < 5 || k + 1 >= FFT_N / 2) continue;
    const float pl = fre[k - 1] * fre[k - 1] + fim[k - 1] * fim[k - 1];
    const float pr = fre[k + 1] * fre[k + 1] + fim[k + 1] * fim[k + 1];
    if (p < pl || p < pr) continue;
    const float amp = 2.0f * sqrtf(p) / sw;
    for (int j = 0; j < 3; j++) {
      if (amp > s.peakMg[j]) {
        for (int m = 2; m > j; m--) { s.peakMg[m] = s.peakMg[m - 1]; s.peakHz[m] = s.peakHz[m - 1]; }
        s.peakMg[j] = amp; s.peakHz[j] = f;
        break;
      }
    }
  }
  for (int b = 0; b < 5; b++) s.band[b] = sqrtf(2.0f * bandSum[b] / (FFT_N * sw2));
  return s;
}

void printSpectrum(const Spectrum& s) {
  Serial.print("Spektrum Z (letzte 1,28 s), stärkste Frequenzen:");
  for (int j = 0; j < 3; j++)
    if (s.peakMg[j] > 0) Serial.printf("  %.1f Hz %.2f mg", s.peakHz[j], s.peakMg[j]);
  Serial.println();
  Serial.print("  Effektivwert je Band:");
  for (int b = 0; b < 5; b++)
    Serial.printf("  %g-%g Hz %.2f mg", BAND_EDGES[b], BAND_EDGES[b + 1], s.band[b]);
  Serial.println();
}

float dominantInBand(const Spectrum& s) {
  for (int j = 0; j < 3; j++)
    if (s.peakHz[j] >= BAND_LO_HZ && s.peakHz[j] <= BAND_HI_HZ) return s.peakHz[j];
  return 0;
}

// ---------- Auswertung ----------
float baseNow()  { return useAll ? baseAll : baseZ; }
float thrNow()   { return FACTORS[factorIdx] * baseNow(); }

void printSettings() {
  Serial.printf("Metrik: %s | Basis %.2f mg | Schwelle x%.1f = %.2f mg\n",
                useAll ? "alle Achsen" : "Z-Achse", baseNow(), FACTORS[factorIdx], thrNow());
}

float median(float* v, uint16_t n) {
  for (uint16_t i = 1; i < n; i++) {            // Einfügesortierung, n = 40
    const float x = v[i];
    int j = i - 1;
    while (j >= 0 && v[j] > x) { v[j + 1] = v[j]; j--; }
    v[j + 1] = x;
  }
  return n % 2 ? v[n / 2] : 0.5f * (v[n / 2 - 1] + v[n / 2]);
}

void startCalibration() {
  calibrating = true;
  calibCount = 0;
  evActive = false;
  Serial.println("\nRuhemessung: 10 s nichts berühren, nicht ziehen ...");
}

void finishCalibration() {
  baseZ   = fmaxf(median(calibZ, CALIB_WINDOWS), 0.05f);
  baseAll = fmaxf(median(calibAll, CALIB_WINDOWS), 0.05f);
  calibrating = false;
  Serial.printf("Ruhe gemessen: Z %.2f mg, alle Achsen %.2f mg (Sensorrauschen laut Datenblatt: Z %.2f mg, alle %.2f mg)\n",
                baseZ, baseAll, expectedZ, expectedAll);
  if (baseZ > 3 * expectedZ)
    Serial.println("  Hinweis: Ruhe deutlich lauter als das Sensorrauschen. Tisch, Kühlschrank, Musik? Das ist die echte Störung am Aufstellort.");
  Serial.printf("Lage (Schwerkraft): X %.0f  Y %.0f  Z %.0f mg", gravX, gravY, gravZ);
  if (fabsf(gravZ) > 850) Serial.println("  -> liegt flach, Z senkrecht. Gut.");
  else Serial.println("  -> Z ist NICHT senkrecht. Sensor flach kleben oder mit 'a' alle Achsen nehmen.");
  printSettings();
  Serial.println("Jetzt ziehen. Ein Zug wird als 'Zug #n' gemeldet, der Ring zeigt den Pegel.\n");
}

void addStat(int8_t l, float lz, float la, uint32_t sw) {
  if (l < 0) return;
  Stat& s = stats[l];
  s.n++;
  s.sumZ += lz; s.sumAll += la;
  s.maxZ = fmaxf(s.maxZ, lz); s.maxAll = fmaxf(s.maxAll, la);
  if (lz > FACTORS[factorIdx] * baseZ) s.overZ++;
  if (la > FACTORS[factorIdx] * baseAll) s.overAll++;
  s.sw += sw;
}

void printEvaluation() {
  Serial.println("\n===== Auswertung =====");
  printSettings();
  Serial.println("Abschnitt   Fenster  Mittel mg  Max mg  Faktor zu Ruhe  über Schwelle  SW-420-Flanken");
  const Stat& r = stats[0];
  const float rMean = r.n ? (float)((useAll ? r.sumAll : r.sumZ) / r.n) : 0;
  for (int l = 0; l < 4; l++) {
    const Stat& s = stats[l];
    if (!s.n) { Serial.printf("%-10s  keine Daten\n", LABELS[l]); continue; }
    const float mean = (float)((useAll ? s.sumAll : s.sumZ) / s.n);
    const float mx = useAll ? s.maxAll : s.maxZ;
    const uint32_t over = useAll ? s.overAll : s.overZ;
    Serial.printf("%-10s  %7lu  %9.2f  %6.2f  %14.1f  %12.0f %%  %14lu\n", LABELS[l],
                  (unsigned long)s.n, mean, mx, rMean > 0 ? mean / rMean : 0.0f,
                  100.0f * over / s.n, (unsigned long)s.sw);
  }
  const Stat& z = stats[1];
  if (r.n && z.n) {
    const float f = (float)((useAll ? z.sumAll : z.sumZ) / z.n) / rMean;
    Serial.printf("Ergebnis: Zug ist im Mittel %.1f-mal so stark wie Ruhe -> ", f);
    if (f >= 3) Serial.println("reicht mit Reserve.");
    else if (f >= 2) Serial.println("reicht knapp (Erkennung geht, wenig Reserve).");
    else if (f >= 1.5f) Serial.println("sehr knapp: nur mit Schwelle x1.5 und ruhigem Tisch. Montage verbessern.");
    else Serial.println("reicht NICHT. Montage prüfen, sonst ADXL355 oder Mikrofon.");
    for (int l = 2; l < 4; l++) {
      const Stat& s = stats[l];
      if (s.n && (useAll ? s.sumAll : s.sumZ) / s.n >= (useAll ? z.sumAll : z.sumZ) / z.n)
        Serial.printf("Achtung: '%s' ist im Mittel so stark wie ein Zug -> Fehlauslösung, wenn es länger als 1 s dauert.\n", LABELS[l]);
    }
  } else {
    Serial.println("Für ein Ergebnis braucht es Daten in 'Ruhe' (0) und 'Zug' (1).");
  }
  Serial.printf("Erkannte Ereignisse: %lu Züge, %lu kurze Stöße. FIFO-Überläufe %lu, I2C-Fehler %lu\n",
                (unsigned long)pullCount, (unsigned long)shockCount,
                (unsigned long)fifoOverruns, (unsigned long)i2cErrors);
  Serial.println("======================\n");
}

void handleEvent(float level, float peak) {
  const bool above = level > thrNow();
  if (!evActive) {
    if (!above) return;
    evActive = true;
    evWins = 1; evBelow = 0;
    evMaxLevel = level; evMaxPeak = peak; evDomHz = 0; evSpecDone = false;
    return;
  }
  evWins++;
  evBelow = level > HOLD_RATIO * thrNow() ? 0 : evBelow + 1;
  evMaxLevel = fmaxf(evMaxLevel, level);
  evMaxPeak  = fmaxf(evMaxPeak, peak);
  if (!evSpecDone && evWins >= SPECTRUM_AT_WINDOW && evBelow == 0 && above) {
    evDomHz = dominantInBand(computeSpectrum());
    evSpecDone = true;
  }
  const bool tooLong = evWins >= MAX_EVENT_WINDOWS;
  if (evBelow < END_WINDOWS && !tooLong) return;

  const uint16_t activeWins = evWins - evBelow;
  const float dur = activeWins * WIN_S;
  if (activeWins >= MIN_PULL_WINDOWS) {
    pullCount++;
    Serial.printf("Zug #%lu: %.2f s, Pegel max %.2f mg (x%.1f), Spitze %.1f mg",
                  (unsigned long)pullCount, dur, evMaxLevel, evMaxLevel / baseNow(), evMaxPeak);
    if (evDomHz > 0) Serial.printf(", Hauptfrequenz %.0f Hz", evDomHz);
    Serial.println(tooLong ? "  (60 s erreicht: Ruhe neu messen mit 'k'?)" : "");
  } else {
    shockCount++;
    Serial.printf("kurzer Stoß: %.2f s, Pegel max %.2f mg (x%.1f), Spitze %.1f mg\n",
                  dur, evMaxLevel, evMaxLevel / baseNow(), evMaxPeak);
  }
  evActive = false;
}

void finishWindow() {
  const float n = win.n;
  const float msZ = win.sz / n, msAll = (win.sx + win.sy + win.sz) / n;
  const float lz = sqrtf(msZ), la = sqrtf(msAll);          // dieses Fenster (Statistik)
  const float sz = sqrtf(0.5f * (msZ + prevMsZ));          // gleitend 0,5 s (Erkennung)
  const float sa = sqrtf(0.5f * (msAll + prevMsAll));
  prevMsZ = msZ; prevMsAll = msAll;
  const float level = useAll ? sa : sz;
  const float peak  = useAll ? win.peakAll : win.peakZ;
  const uint32_t e = swEdges;                 // 32-bit-Lesen ist atomar
  const uint32_t sw = e - swEdgesSeen;
  swEdgesSeen = e;

  if (calibrating) {
    calibZ[calibCount] = lz;
    calibAll[calibCount] = la;
    const float k = 1.0f / (calibCount + 1);  // laufender Mittelwert
    gravX += (win.gx / n - gravX) * k;
    gravY += (win.gy / n - gravY) * k;
    gravZ += (win.gz / n - gravZ) * k;
    addStat(0, lz, la, sw);                   // Ruhemessung zählt als "Ruhe"
    calibCount++;
    ringShowCalib();
    if (calibCount >= CALIB_WINDOWS) finishCalibration();
  } else {
    const bool above = level > thrNow();
    addStat(label, lz, la, sw);
    handleEvent(level, peak);
    ringShowLevel(level / baseNow(), above);

    secWins++;
    secMaxLevel = fmaxf(secMaxLevel, level);
    secMaxPeak  = fmaxf(secMaxPeak, peak);
    secSw += sw;
    secAbove |= above;
    if (outMode == Out::Plot) {
      Serial.printf("Pegel:%.3f Schwelle:%.3f Basis:%.3f\n", level, thrNow(), baseNow());
    }
    if (secWins >= 4) {
      if (outMode == Out::Normal) {
        Serial.printf("%5lu s | Pegel %6.2f mg (x%4.1f) | Spitze %6.1f mg | SW-420 %3lu | %-8s %s\n",
                      (unsigned long)(millis() / 1000), secMaxLevel, secMaxLevel / baseNow(),
                      secMaxPeak, (unsigned long)secSw, label >= 0 ? LABELS[label] : "-",
                      secAbove ? "<< über Schwelle" : "");
      }
      secWins = 0; secMaxLevel = 0; secMaxPeak = 0; secSw = 0; secAbove = false;
    }
  }
  win = Window();
}

void processSample(float x, float y, float z) {
  zBuf[zPos] = z;
  zPos = (zPos + 1) % FFT_N;
  const float bx = fx.step(x), by = fy.step(y), bz = fz.step(z);
  if (++samplesSeen <= SETTLE_SAMPLES) return;   // Filter schwingen ein
  if (outMode == Out::Raw) Serial.printf("%.3f\n", bz);
  win.sx += bx * bx; win.sy += by * by; win.sz += bz * bz;
  win.peakZ = fmaxf(win.peakZ, fabsf(bz));
  win.peakAll = fmaxf(win.peakAll, sqrtf(bx * bx + by * by + bz * bz));
  win.gx += x; win.gy += y; win.gz += z;
  if (++win.n >= WIN_SAMPLES) finishWindow();
}

void readFifo() {
  uint8_t fs = 0;
  if (!readReg(REG_FIFO_SAMPLES, fs)) { i2cErrors++; return; }
  if (fs & 0x40) fifoOverruns++;
  const uint8_t count = fs & 0x3F;              // DIFF5..0, 32 = voll
  for (uint8_t i = 0; i < count; i++) {
    uint8_t b[6];
    if (!readRegs(REG_OUT_X_L, b, 6)) { i2cErrors++; return; }
    const int16_t rx = (int16_t)(b[1] << 8 | b[0]) >> 2;
    const int16_t ry = (int16_t)(b[3] << 8 | b[2]) >> 2;
    const int16_t rz = (int16_t)(b[5] << 8 | b[4]) >> 2;
    processSample(rx * MG_PER_LSB, ry * MG_PER_LSB, rz * MG_PER_LSB);
  }
}

// erwartetes Sensorrauschen im Nutzband aus der Filterkurve
void computeExpectedNoise() {
  BandFilter f;
  f.init();
  float enbw = 0;
  const float df = 0.25f;
  for (float fr = df / 2; fr < FS_HZ / 2; fr += df) enbw += f.hp.gain2(fr) * f.lp.gain2(fr) * df;
  expectedZ = NOISE_UG_RTHZ / 1000.0f * sqrtf(enbw);
  expectedAll = expectedZ * sqrtf(3.0f);
  Serial.printf("Nutzband %.0f-%.0f Hz (Rauschbandbreite %.0f Hz)\n", BAND_LO_HZ, BAND_HI_HZ, enbw);
}

void printHelp() {
  Serial.println("Befehle (Taste + Enter):");
  Serial.println("  0 Ruhe   1 Zug   2 Schlauch anfassen   3 Tisch antippen   x kein Abschnitt");
  Serial.println("  e Auswertung   f Spektrum jetzt   k Ruhe neu messen   a Z/alle Achsen");
  Serial.println("  + / - Schwelle   p Plotter   r Rohdaten 400 Hz   q nur Ereignisse   h Hilfe");
  Serial.println("Tipp: '1' kurz vor dem Zug, '0' direkt danach. Am Ende 'e'.");
}

void handleSerial() {
  while (Serial.available()) {
    const char c = Serial.read();
    switch (c) {
      case '0': case '1': case '2': case '3':
        label = c - '0';
        Serial.printf("-> Abschnitt: %s\n", LABELS[(int)label]);
        break;
      case 'x': label = -1; Serial.println("-> kein Abschnitt"); break;
      case 'e': printEvaluation(); break;
      case 'f': printSpectrum(computeSpectrum()); break;
      case 'k': startCalibration(); break;
      case 'a':
        useAll = !useAll;
        evActive = false;
        printSettings();
        break;
      case '+': if (factorIdx + 1 < N_FACTORS) factorIdx++; printSettings(); break;
      case '-': if (factorIdx > 0) factorIdx--; printSettings(); break;
      case 'p': outMode = outMode == Out::Plot ? Out::Normal : Out::Plot; break;
      case 'r': outMode = outMode == Out::Raw ? Out::Normal : Out::Raw; break;
      case 'q':
        outMode = outMode == Out::Quiet ? Out::Normal : Out::Quiet;
        Serial.println(outMode == Out::Quiet ? "-> nur Ereignisse" : "-> Sekundenzeilen an");
        break;
      case 'h': printHelp(); break;
      default: break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);   // USB-CDC des ESP32-C3 braucht einen Moment
  Serial.println("\n=== B0 Beschleunigungstest LIS2DW12 ===");
  ring.begin();
  ring.setBrightness(LED_BRIGHT);
  ring.clear();
  ring.show();

  if (USE_SW420) {
    pinMode(PIN_SW420, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_SW420), onSw420, CHANGE);
  }
  fx.init(); fy.init(); fz.init();
  for (int i = 0; i < FFT_N; i++) zBuf[i] = 0;
  computeExpectedNoise();
  sensorOk = initSensor();
  printHelp();
  startCalibration();
}

void loop() {
  handleSerial();
  readFifo();     // FIFO fasst 32 Werte = 80 ms; loop läuft viel schneller
  delay(5);
}
