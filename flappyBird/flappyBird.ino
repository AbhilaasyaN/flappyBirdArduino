// headers
 // built in arduino I2C library
#include <Wire.h>
// libraries for oled
#include <Adafruit_GFX.h> // graphics core library
#include <Adafruit_SSD1306.h> // OLED driver library
// resolution/size of the oled display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BUTTON_PIN 2 // the button we use is digital pin 2
// &Wire means that we use the I2C, and the -1 means that the game is not including a reset pin, it might just start over again by itself
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// My variables (defines the variables for the button logic of pressing, leaving the button, and holding the button)
// a pull-up configuration is used, so high means that it is not pressed and low means that it is pressed. active low
bool buttonState = HIGH;
int pressCount = 0; // this is for debugging, to tell that the button is wokring

// button to help draw screen
void drawScreen(bool buttonPressed)
{
  display.clearDisplay(); // start with a fresh screen
  display.setTextColor(SSD1306_WHITE); // default color for the on pixels
  display.setTextSize(2); // the size of the text
  display.setCursor(0,0); // start the message here

  if(buttonPressed)
  {
    display.println("FLAP!"); 
  }
  else
  {
    display.println("READY");
  }

  display.setTextSize(1);
  display.setCursor(0,40);
  display.print("Presses: ");
  display.println(pressCount);
  display.display();
  // counter text to see if the button presses are working
}

// Testing the OLED setup/connection:
void setup() {
  // put your setup code here, to run once:
  Wire.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) // if this happens, wiring address is wrong
  {
    while(true);
  }

  drawScreen(false);
}

//   display.clearDisplay();

//   //Drawing the text
//   display.setTextSize(2);
//   display.setTextColor(SSD1306_WHITE);
//   display.setCursor(10,20);
//   display.println("HELLO");

//   display.display(); // push buffer to screen

// }

void loop() {
  // put your main code here, to run repeatedly:
  // reads the current button state
  bool currentState = digitalRead(BUTTON_PIN);
  if(buttonState == HIGH && currentState == LOW)
  {
    pressCount++;
    drawScreen(true);
  }
  if(buttonState == LOW && currentState == HIGH)
  {
    drawScreen(false); // sets it back to ready position
  }

  buttonState = currentState;
}
