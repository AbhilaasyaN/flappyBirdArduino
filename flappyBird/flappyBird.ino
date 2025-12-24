#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

#define BUTTON_PIN 2

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --------- Debounce variables (same idea as before) ----------
bool stableState = HIGH;
bool lastReading = HIGH;
unsigned long lastChangeMs = 0;
const unsigned long debounceMs = 25;

// --------- Game timing ----------
unsigned long lastUpdateMs = 0;
const unsigned long dtMs = 20;   // 20 ms = 50 updates per second

// --------- Bird physics ----------
float birdY = 20;               // vertical position (pixels)
float birdV = 0;                // vertical velocity (pixels per update)
const float gravity = 0.35;      // acceleration downward
const float flapImpulse = -5.5;  // upward kick (negative = up)

// Bird drawing
const int birdX = 25;
const int birdW = 6;
const int birdH = 6;

void render() {
  display.clearDisplay();

  // Draw bird as a filled rectangle
  display.fillRect(birdX, (int)birdY, birdW, birdH, SSD1306_WHITE);

  // Optional: draw ground line
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);

  display.display();
}

// Returns true only when a NEW press event happens (HIGH->LOW), debounced
bool buttonPressedEvent() {
  bool event = false;

  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) {
    lastChangeMs = millis();
    lastReading = reading;
  }

  if ((millis() - lastChangeMs) > debounceMs) {
    if (stableState != reading) {
      stableState = reading;

      // Press event happens when stableState becomes LOW
      if (stableState == LOW) {
        event = true;
      }
    }
  }

  return event;
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true);
  }

  lastUpdateMs = millis();
  render();
}

void loop() {
  // Handle flap input (event-based)
  if (buttonPressedEvent()) {
    birdV = flapImpulse;  // reset velocity upward
  }

  // Fixed timestep update
  unsigned long now = millis();
  if (now - lastUpdateMs >= dtMs) {
    lastUpdateMs += dtMs;

    // Physics update
    birdV += gravity;
    birdY += birdV;

    // Clamp to screen bounds
    if (birdY < 0) {
      birdY = 0;
      birdV = 0;
    }
    if (birdY > SCREEN_HEIGHT - birdH) {
      birdY = SCREEN_HEIGHT - birdH;
      birdV = 0;
    }

    // Draw frame
    render();
  }
}


