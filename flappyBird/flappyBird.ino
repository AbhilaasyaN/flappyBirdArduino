#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---- OLED config ----
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C   // most common; change to 0x3D if needed

// ---- Button config ----
#define BUTTON_PIN 2     // your wiring: D2
// With INPUT_PULLUP: HIGH = released, LOW = pressed

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---- Button state + debounce ----
bool stableState = HIGH;           // debounced, trusted button state
bool lastReading = HIGH;           // last raw read
unsigned long lastChangeMs = 0;    // when the raw read last changed
const unsigned long debounceMs = 25;

int pressCount = 0;

void render() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(stableState == LOW ? "FLAP!" : "READY");

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Presses: ");
  display.println(pressCount);

  display.display();
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true); // stop if OLED init fails
  }

  render(); // initial screen
}

void loop() {
  // 1) Read the raw (possibly bouncy) input
  bool reading = digitalRead(BUTTON_PIN);

  // 2) If raw reading changed, reset debounce timer
  if (reading != lastReading) {
    lastChangeMs = millis();
    lastReading = reading;
  }

  // 3) If the reading has been stable for long enough, accept it
  if ((millis() - lastChangeMs) > debounceMs) {
    // If debounced state changes, we have a real event
    if (stableState != reading) {
      stableState = reading;

      // Count only on a PRESS event (HIGH -> LOW)
      if (stableState == LOW) {
        pressCount++;
      }

      // Update OLED whenever the stable state changes
      render();
    }
  }
}
