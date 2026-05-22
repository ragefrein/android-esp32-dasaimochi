#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <time.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TOUCH_PIN 23   // pin input dari sensor TTP22

// ===== WiFi =====
const char* ssid = "AndroidAP_2821";
const char* password = "pooja28";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

const unsigned char wifi_icon [] PROGMEM = {
  0x1C, 0x00,
  0x22, 0x00,
  0x41, 0x00,
  0x94, 0x80,
  0x22, 0x40,
  0x41, 0x20,
  0x80, 0x10,
  0x00, 0x00
};

// ===== Bitmap animasi =====
const unsigned char epd_bitmap_00 [] PROGMEM = 
const unsigned char* frames[90] = { epd_bitmap_00 };
const int numFrames = sizeof(frames) / sizeof(frames[0]);
int frame = 0;

int mode = 0;
bool lastTouchState = false;
unsigned long lastDebounceTime = 0;

// Array nama hari
const char* hariIndo[] = { "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu" };

// ===== Mata kotak =====
int leftEyeX = 20;
int rightEyeX = 80;
int eyeY = 20;
int eyeWidth = 16;
int eyeHeight = 12;
int pupilSize = 4;

int blinkCounter = 0;
bool blinking = false;

void drawEye(int x, int y, int pupilOffsetX, bool closed) {
  if (closed) {
    display.drawLine(x, y + eyeHeight/2, x + eyeWidth, y + eyeHeight/2, WHITE);
  } else {
    display.drawRect(x, y, eyeWidth, eyeHeight, WHITE);
    display.fillRect(x + eyeWidth/2 - pupilSize/2 + pupilOffsetX, y + eyeHeight/2 - pupilSize/2, pupilSize, pupilSize, WHITE);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  // ===== WiFi Connect =====
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 25);
  display.print("Menghubungkan WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("\nWiFi terhubung!");
}

void loop() {
  bool touchState = digitalRead(TOUCH_PIN);

  if (touchState && !lastTouchState && millis() - lastDebounceTime > 600) {
    mode++;
    if (mode > 2) mode = 0;
    lastDebounceTime = millis();
  }
  lastTouchState = touchState;

  display.clearDisplay();

  if (mode == 0) {
    // ===== MODE 0: Animasi + Mata =====
    
    // Kedip otomatis
    if (blinkCounter == 50) blinking = true;
    if (blinkCounter == 55) {
      blinking = false;
      blinkCounter = 0;
    }
    blinkCounter++;

    int pupilMove = sin(millis() / 300.0) * 4; // ±4 px

    // Gambar 2 mata
    drawEye(leftEyeX, eyeY, pupilMove, blinking);
    drawEye(rightEyeX, eyeY, pupilMove, blinking);

    // Gambar animasi bitmap lama di bawah mata
    display.drawBitmap(0, 35, frames[frame], 128, 29, WHITE);
    frame++;
    if (frame >= numFrames) frame = 0;

    display.display();
    delay(80);

  } else if (mode == 1) {
    // ===== MODE 1: Hallo Rachel =====
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 25);
    display.print("Hallo");
    display.setCursor(10, 45);
    display.print("Rachel");
    display.display();

  } else if (mode == 2) {
    // ===== MODE 2: Jam + Hari + Tanggal =====
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      display.setTextSize(1);
      display.setCursor(10, 30);
      display.print("Gagal ambil waktu");
      display.display();
      return;
    }

    char jamStr[10];
    strftime(jamStr, sizeof(jamStr), "%H:%M", &timeinfo);

    char tanggalStr[15];
    strftime(tanggalStr, sizeof(tanggalStr), "%d/%m/%Y", &timeinfo);

    int dayIndex = timeinfo.tm_wday;
    const char* hari = hariIndo[dayIndex];

    display.drawLine(0, 10, 128, 10, WHITE);
    display.setTextSize(1);
    display.setCursor(2, 0);

    if (WiFi.status() == WL_CONNECTED)
      display.drawBitmap(115, 0, wifi_icon, 10, 8, WHITE);

    display.setTextSize(3);
    display.setCursor(15, 25);
    display.print(jamStr);

    display.setTextSize(1);
    display.setCursor(15, 55);
    display.print(hari);
    display.print(", ");
    display.print(tanggalStr);

    display.display();
    delay(500);
  }
}