#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -------------------- OLED + board config --------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

#define BUTTON_PIN 2   // your wiring: D2 (PWM-capable is still digital)

// Create display object (I2C via Wire, no reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------------------- Debounce (same approach as before) --------------------
// stableState = debounced, trusted button state
// reading/lastReading = raw reads (can bounce)
bool stableState = HIGH;
bool lastReading = HIGH;
unsigned long lastChangeMs = 0;
const unsigned long debounceMs = 25;

// -------------------- Game timing --------------------
// We update at a fixed rate so the game feels consistent
unsigned long lastUpdateMs = 0;
const unsigned long dtMs = 20; // 20ms => 50 updates/second

// -------------------- Bird physics --------------------
// We'll treat the bird as a rectangle (hitbox = what collides)
float birdY = 20;      // vertical position (pixels)
float birdV = 0;       // vertical velocity (pixels per update)

const float gravity = 0.35;      // pulls down every update
const float flapImpulse = -5.5;  // upward velocity on press (negative = up)

// Bird drawing/hitbox dimensions
const int birdX = 25;  // fixed x position (Flappy Bird style)
const int birdW = 6;
const int birdH = 6;

// -------------------- Pipe settings --------------------
// Pipes are two rectangles: top pipe and bottom pipe, with a gap in between.
int pipeX = SCREEN_WIDTH;  // start at right edge
const int pipeW = 16;      // width of pipe
const int gapH = 22;       // height of the gap (bigger = easier)

// gapY is the y position where the gap starts (top of gap)
int gapY = 20;

// How fast pipes move left (pixels per update step)
const int pipeSpeed = 2;

// Score rules: +1 when pipe passes bird
int score = 0;
bool scoredThisPipe = false;

// -------------------- Game state (TEMP minimal) --------------------
// You said you'll add start screen and full game state after.
// For now, we just freeze on crash and show a message.
bool crashed = false;

// -------------------- Utility: simple pseudo-random gap --------------------
// Arduino has random(), but you usually seed it with randomSeed().
// We'll seed it once using analogRead noise from an unconnected pin.
void resetPipe() {
  pipeX = SCREEN_WIDTH; // respawn on the right
  // Make a new random gap location:
  // Keep it within screen bounds so pipes draw correctly.
  int minGapY = 8;
  int maxGapY = SCREEN_HEIGHT - gapH - 8;
  gapY = random(minGapY, maxGapY + 1);

  scoredThisPipe = false;
}

// -------------------- Debounced button press event --------------------
// Returns true ONLY once per physical press (HIGH->LOW), after debouncing.
bool buttonPressedEvent() {
  bool event = false;

  bool reading = digitalRead(BUTTON_PIN);

  // If raw reading changed, reset debounce timer
  if (reading != lastReading) {
    lastChangeMs = millis();
    lastReading = reading;
  }

  // If stable long enough, accept it as real
  if ((millis() - lastChangeMs) > debounceMs) {
    if (stableState != reading) {
      stableState = reading;

      // Press event = stableState becomes LOW
      if (stableState == LOW) {
        event = true;
      }
    }
  }

  return event;
}

// -------------------- Collision helper --------------------
// Check if two axis-aligned rectangles overlap (AABB collision).
// Each rectangle is defined by (x, y, w, h).
bool rectsOverlap(int ax, int ay, int aw, int ah,
                  int bx, int by, int bw, int bh) {
  // If one rectangle is completely to the left/right/up/down of the other, no overlap
  if (ax + aw <= bx) return false;
  if (bx + bw <= ax) return false;
  if (ay + ah <= by) return false;
  if (by + bh <= ay) return false;
  return true;
}

// -------------------- Render everything --------------------
void renderFrame() {
  display.clearDisplay();

  // Draw top pipe: from y=0 down to gapY
  display.fillRect(pipeX, 0, pipeW, gapY, SSD1306_WHITE);

  // Draw bottom pipe: from (gapY + gapH) down to bottom of screen
  int bottomPipeY = gapY + gapH;
  display.fillRect(pipeX, bottomPipeY, pipeW, SCREEN_HEIGHT - bottomPipeY, SSD1306_WHITE);

  // Draw bird
  display.fillRect(birdX, (int)birdY, birdW, birdH, SSD1306_WHITE);

  // Draw score
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Score: ");
  display.print(score);

  // If crashed, overlay message
  if (crashed) {
    display.setTextSize(2);
    display.setCursor(10, 24);
    display.print("CRASH");

    display.setTextSize(1);
    display.setCursor(8, 50);
    display.print("Press to reset");
  }

  display.display();
}

// -------------------- Reset the whole game (we'll refine later) --------------------
void resetGame() {
  birdY = 20;
  birdV = 0;
  score = 0;
  crashed = false;
  resetPipe();
  renderFrame();
}

// -------------------- Setup --------------------
void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true);
  }

  // Seed randomness (helps pipe gaps vary each run)
  randomSeed(analogRead(A0));

  resetGame();
  lastUpdateMs = millis();
}

// -------------------- Main loop --------------------
void loop() {
  // Handle input:
  // - If not crashed: flap makes bird jump
  // - If crashed: press resets game
  if (buttonPressedEvent()) {
    if (!crashed) {
      birdV = flapImpulse;
      if (birdV <-3.0)
      {
        birdV = -3.0;
      }
    } else {
      resetGame();
    }
  }

  // Fixed timestep update: only update game logic every dtMs
  unsigned long now = millis();
  if (now - lastUpdateMs >= dtMs) {
    lastUpdateMs += dtMs;

    // If crashed, freeze physics/movement (but still render so message stays)
    if (crashed) {
      renderFrame();
      return;
    }

    // ---------------- Bird physics update ----------------
    birdV += gravity;
    birdY += birdV;

    // Clamp bird to screen bounds
    if (birdY < 0) {
      birdY = 0;
      birdV = 0;
    }
    if (birdY > SCREEN_HEIGHT - birdH) {
      birdY = SCREEN_HEIGHT - birdH;
      birdV = 0;
    }

    // ---------------- Pipe movement ----------------
    pipeX -= pipeSpeed;

    // If pipe moved off the left side, respawn it
    if (pipeX + pipeW < 0) {
      resetPipe();
    }

    // ---------------- Scoring ----------------
    // We count a point when the pipe has fully passed the bird's x position.
    if (!scoredThisPipe && (pipeX + pipeW) < birdX) {
      score++;
      scoredThisPipe = true;
    }

    // ---------------- Collision detection ----------------
    // Bird rectangle
    int bx = birdX;
    int by = (int)birdY;

    // Top pipe rectangle
    int topX = pipeX;
    int topY = 0;
    int topW = pipeW;
    int topH = gapY;

    // Bottom pipe rectangle
    int botX = pipeX;
    int botY = gapY + gapH;
    int botW = pipeW;
    int botH = SCREEN_HEIGHT - botY;

    // If bird overlaps either pipe -> crash
    bool hitTop = rectsOverlap(bx, by, birdW, birdH, topX, topY, topW, topH);
    bool hitBot = rectsOverlap(bx, by, birdW, birdH, botX, botY, botW, botH);

    if (hitTop || hitBot) {
      crashed = true;
    }

    // ---------------- Draw the frame ----------------
    renderFrame();
  }
}


