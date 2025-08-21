#include <Adafruit_NeoPixel.h>  // Library to control NeoPixel LEDs
#include <Keyboard.h>           // Library to emulate keyboard input
#include "Chatpad.h"            // Library for interfacing with the Xbox 360 Chatpad

// Define the pin and number of NeoPixels used
#define NEOPIXEL_PIN 17         // Digital pin connected to the NeoPixel
#define NUM_PIXELS 1            // Only 1 LED in this example

// Create the NeoPixel object; using GRB color order and an 800 KHz signal
Adafruit_NeoPixel neoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Define which serial port to use for the Chatpad communication
#define CHATPAD_SERIAL Serial1

// Define LED color constants using macros. These use the neoPixel.Color() method:
#define COLOR_BLUE       (neoPixel.Color(0, 0, 255))
#define COLOR_GREEN      (neoPixel.Color(0, 255, 0))
#define COLOR_YELLOW     (neoPixel.Color(255, 255, 0))
#define COLOR_RED        (neoPixel.Color(255, 0, 0))
#define COLOR_OFF        (neoPixel.Color(0, 0, 0))
#define COLOR_HEARTBEAT  (neoPixel.Color(20, 20, 20))  // Dim white for heartbeat debugging

// Create an instance of the Chatpad
Chatpad chatpad;

// Global variables for heartbeat and debug LED timing
unsigned long lastHeartbeat = 0;
unsigned long heartbeatStart = 0;
bool heartbeatOn = false;
unsigned long debugLEDExpire = 0;

//-----------------------------------------------------------------
// Function: debugLED
// Purpose: Flash the LED with a specific color for debugging purposes.
// Parameters:
//   - color: The color to display (use one of the COLOR_* macros)
//   - duration: How long to display the color (default: 100 ms)
//-----------------------------------------------------------------
void debugLED(uint32_t color, uint16_t duration = 100) {
  neoPixel.setPixelColor(0, color);
  neoPixel.show();
  debugLEDExpire = millis() + duration;
}

//-----------------------------------------------------------------
// Function: handleKey
// Purpose: Called on key events from the Chatpad.
// Parameters:
//   - pad: Reference to the Chatpad object
//   - key: Key code from the Chatpad
//   - event: Event type (Down for key press, Up for key release)
//-----------------------------------------------------------------
void handleKey(Chatpad &pad, Chatpad::keycode_t key, Chatpad::eventtype_t event) {
  char asciiKey = pad.toAscii(key);  // Convert keycode to ASCII (if possible)
  
  if (event == Chatpad::Down) {  // On key press
    if (asciiKey) {
      // Send key press to the computer
      Keyboard.press(asciiKey);
      Serial.print("Key pressed: ");
      Serial.println(asciiKey);

      // Set LED color based on key type
      unsigned char ch = static_cast<unsigned char>(asciiKey);
      if (isalpha(ch))
        neoPixel.setPixelColor(0, COLOR_BLUE);    // Blue for letters
      else if (isdigit(ch))
        neoPixel.setPixelColor(0, COLOR_GREEN);     // Green for digits
      else
        neoPixel.setPixelColor(0, COLOR_YELLOW);    // Yellow for other ASCII characters
    } else {
      // For non-ASCII keys, log the code and flash red for debugging
      Serial.print("Non-ASCII key pressed, code: ");
      Serial.println(key, HEX);
      neoPixel.setPixelColor(0, COLOR_RED);
    }
    neoPixel.show();
  } 
  else if (event == Chatpad::Up) {  // On key release
    if (asciiKey)
      Keyboard.release(asciiKey);
    // Turn off the LED when the key is released
    neoPixel.setPixelColor(0, COLOR_OFF);
    neoPixel.show();
  }
}

//-----------------------------------------------------------------
// Function: setup
// Purpose: Initializes Serial communication, the NeoPixel LED,
//          Chatpad, and Keyboard. Also does initial LED debugging.
//-----------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  
  // Initialize the NeoPixel library and set the brightness
  neoPixel.begin();
  neoPixel.setBrightness(50);
  neoPixel.show();
  
  // LED Debugging: Blink the LED twice to indicate successful initialization
  for (int i = 0; i < 2; i++) {
    neoPixel.setPixelColor(0, COLOR_BLUE);
    neoPixel.show();
    delay(200);
    neoPixel.setPixelColor(0, COLOR_OFF);
    neoPixel.show();
    delay(200);
  }
  
  // Initialize the Chatpad serial connection and its key callback
  CHATPAD_SERIAL.begin(19200);
  chatpad.init(CHATPAD_SERIAL, handleKey);
  
  // Initialize Keyboard functionality (required for sending key presses)
  Keyboard.begin();
}

//-----------------------------------------------------------------
// Function: loop
// Purpose: Poll the Chatpad for key events and run a heartbeat LED
//          debug pattern every second.
//-----------------------------------------------------------------
void loop() {
  chatpad.poll();  // Poll Chatpad for key events

  unsigned long now = millis();

  // Turn off debug LED after the requested duration
  if (debugLEDExpire && now >= debugLEDExpire) {
    neoPixel.setPixelColor(0, COLOR_OFF);
    neoPixel.show();
    debugLEDExpire = 0;
  }

  // Heartbeat LED Debugging: Blink a dim white LED every 1000 ms
  if (!heartbeatOn && now - lastHeartbeat >= 1000) {
    neoPixel.setPixelColor(0, COLOR_HEARTBEAT);
    neoPixel.show();
    heartbeatOn = true;
    heartbeatStart = now;
    lastHeartbeat = now;
  }

  // Turn off the heartbeat after 50 ms
  if (heartbeatOn && now - heartbeatStart >= 50) {
    neoPixel.setPixelColor(0, COLOR_OFF);
    neoPixel.show();
    heartbeatOn = false;
  }
}
