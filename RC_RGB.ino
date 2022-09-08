//RC controlled Neopixel RGB program
//Peter Gautier 2019.   gautier.p62@gmail.com

#include <FastLED.h>   //https://github.com/FastLED/FastLED
#include <EEPROM.h> 
#include "PWM.hpp"     //https://github.com/xkam1x/Arduino-PWM-Reader
PWM ch1(2); // Setup pin 2 for input
PWM ch2(3); // Setup pin 3 for input
//PWM ch3(18); // Setup pin 18 for input
//PWM ch4(4); // Setup pin 19 for input
//PWM ch5(5); // Setup pin 20 for input
//PWM ch6(21); // Setup pin 21 for input

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    9
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    30
CRGB leds[NUM_LEDS];
#define FRAMES_PER_SECOND  120

//  EEPROM addresses
const int EEPROM_HUE = 0;    //1 byte
const int EEPROM_BRIGHTNESS = 2;        //1 byte
const int EEPROM_SATURATION = 3;        //1 byte
const int EEPROM_CYCLE = 4;             //1 byte
const int EEPROM_MAGIC = 5;             //1 byte


// Default values for parameters
byte hue = 0;
byte brightness = 255;
byte saturation = 255;
byte cycle = 100;
byte magic = 0;
int brstep = 1;  
byte br = 0;

int pos = 0; //position for servo


void setup() { 
  //myservo.attach(9);

    ch1.begin(true); // ch1 on pin 2 reading PWM HIGH duration
    ch2.begin(true); // ch2 on pin 3 reading PWM HIGH duration
    delay(10);
//    ch3.begin(true); // ch3 on pin 18 reading PWM HIGH duration
//    ch4.begin(true); // ch4 on pin 19 reading PWM HIGH duration
//    ch5.begin(true); // ch5 on pin 20 reading PWM HIGH duration
//    ch6.begin(true); // ch6 on pin 21 reading PWM HIGH duration

Serial.begin(57600); // Pour a bowl of Serial

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);    // tell FastLED about the LED strip configuration
  for (int i=0;i<NUM_LEDS;i++) leds[i] = CHSV( hue, saturation, brightness);  //hue, stauration, brightness
  FastLED.setBrightness(255);  // set master brightness control
  FastLED.show();
  br = brightness;

}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
int v1,v2;
void loop() {

  v1 = ch1.getValue();
  v2 = ch2.getValue();

  Serial.print("Channel 1:"); Serial.print(v1); // each channel
  Serial.print("  Channel 2:");Serial.println(v2);
  
  if (v2<1200) v2 = 1200;
  if (v2>1750) v2 = 1750;
  brightness = map(v2,1200,1750,0,255);

  
//Serial.print("Channel 3:");Serial.println(ch3);

  if (v1<1100) {
     gPatterns[gCurrentPatternNumber]();
     gHue++;
     //rainbow();
    }
  else {
    saturation = 255; if (v1>1850) saturation = 0; //white color
    if (v1>1850) v1 = 1850;
    if (v1<1100) v1 = 1100;
    gHue = map(v1,1100,1850,0,255);
    for (int i=0;i<NUM_LEDS;i++) leds[i] = CHSV(gHue, saturation, 255);  //hue, stauration, brightness
    }
    
  FastLED.setBrightness(brightness);
  FastLED.show();
  delay(30);
  EVERY_N_SECONDS( 50 ) { nextPattern(); } // change patterns periodically

}  //end loop


#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 2);
}

void rainbowWithGlitter() {
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon() {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

#define FLASHDELAY 200
void flash(byte num) {      //flashing num times
  FastLED.setBrightness(0);  // set master brightness control
  delay(FLASHDELAY);
  for (int j=0;j<num+1;j++) {
    for (int i=0;i<6;i++) leds[i] = CHSV( 100, 0, 255);  //hue, stauration, brightness
    FastLED.setBrightness(255);  // set master brightness control
    FastLED.show();
    delay(FLASHDELAY);
    FastLED.setBrightness(0);  // set master brightness control
    FastLED.show();
    delay(FLASHDELAY);
  }
}





/* servo positioning
 * if ((ch3 >= 1500) && (ch3 <= 1600)){// the center postion for controller
  pos = 90;//set servo to center
  myservo.write(pos); // set to pos which is 90
  }
else{
  Serial.println("not between");
  for(pos = 0; pos < 180; pos += 1) // goes from 0 degrees to 180 degrees
    { // in steps of 1 degree
    myservo.write(pos); // tell servo to go to position in variable 'pos'
    delay(1); // waits 1ms for the servo to reach the position
    } //end for

for(pos = 180; pos>=1; pos-=1) // goes from 180 degrees to 0 degrees
  {

  myservo.write(pos); // tell servo to go to position in variable 'pos'
  delay(1); // waits 1ms for the servo to reach the position
  }

 * */
 
