#include <Adafruit_NeoPixel.h>

#define PIN 3 // A0 on Feather
#define NUMPIXELS 8 // NeoPixels count

int indicatorStatus = 0; //needs to be implemented to be updated from rx tx from wixel from Worn Device
long tmpArray[] = {0, 0, 0, 0, 0};                 // an array of data for Temp Sensor Matrix
uint8_t RFin = 1;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

uint32_t red = pixels.Color(255, 0, 0);
uint32_t green = pixels.Color(0, 255, 0);
uint32_t blue = pixels.Color(0, 0, 255);
uint32_t orange = pixels.Color(255, 165, 0);
uint32_t magenta = pixels.Color(255, 0, 255);

#define DELAYVAL 50 // Time (in milliseconds) to pause between pixels

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
}

void loop() {
  if (Serial.available()) {        // If anything comes in Serial (USB),
    Serial1.write(Serial.read());  // read it and send it out Serial1 (pins 0 & 1)
  }

  if (Serial1.available()) {       // If anything comes in Serial1 (pins 0 & 1)
    RFin = Serial1.read();
  }
  Serial.print("RF Monitor Device: ");
  Serial.println(RFin);

  if (RFin == 48){ //Status IDLE
    updateColor(green);
  } else if (RFin == 49){ //Status 1
    updateColor(orange);
  } else if (RFin == 50){ //Status 2
    updateColor(magenta);
  } else { //Status ERROR
    pixels.clear(); // Set all pixel colors to 'off'
    pixels.show();
  }
}

void updateColor(uint32_t color) {
  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    pixels.setPixelColor(i, color);
    pixels.show();   // Send the updated pixel colors to the hardware.
  }
}
