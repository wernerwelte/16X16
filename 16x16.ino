#include <FastLED.h>

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define DATA_PIN    8
#define BUTTON1_PIN    2
#define BUTTON2_PIN    7

#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    256
CRGB leds[NUM_LEDS+1];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

// Params for width and height
const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 16;

// Param for different pixel layouts
const bool    kMatrixSerpentineLayout = true;

const int ledPin =  13;      // the number of the LED pin

volatile int buttonState1, buttonState2, state_1 = 0,state_1_new = 0, state_2 = 1;         // variable for reading the pushbutton status

static uint8_t hue = 0;

typedef void (*SimplePatternList[])();


uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0, c = 1, d = 1; // rotating "base color" used by many of the patterns


void cyclon();
void juggle();
void bpm();
void sinelon();
void confetti(); 
void rainbowWithGlitter(); 
void rainbow(); 
void patternCycle();

SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, cyclon, farbflash };

void setup() 
{
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  
  pinMode(BUTTON1_PIN, INPUT);  digitalWrite(BUTTON1_PIN, HIGH);
  pinMode(BUTTON2_PIN, INPUT);  digitalWrite(BUTTON2_PIN, HIGH);
  
  delay(3000); // 3 second delay for recovery 
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), isr_button1 , FALLING);
}


  
void loop()
{
  FastLED.show();  
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow


if( state_1_new == 1 )
{
  detachInterrupt(digitalPinToInterrupt(BUTTON1_PIN));

  state_1_new  = 0;
  if ( state_1++ > 8 ) { state_1 = 1; }
  Serial.println(state_1);
  for(unsigned long i=0; i<100000; i++);
  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), isr_button1 , FALLING);
}


if ( state_2 == 1 )
{
     if ( state_1 == 1 ) { farbflash()            ;}
else if ( state_1 == 2 ) { rainbow( )             ;}
else if ( state_1 == 3 ) { rainbowWithGlitter( )  ;}
else if ( state_1 == 4 ) { confetti( )            ;}
else if ( state_1 == 5 ) { sinelon ( )            ;}
else if ( state_1 == 6 ) { juggle( )              ;}
else if ( state_1 == 7 ) { bpm( )                 ;}
else if ( state_1 == 8 ) { cyclon()               ;}
}
else if ( state_2 == 2 ) { patternCycle()         ;}
}

void patternCycle()
{
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
  gPatterns[gCurrentPatternNumber]();
}

void nextPattern()
{
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}


void rainbow() 
{
  fill_rainbow( leds, NUM_LEDS, gHue, 16);  // FastLED's built-in rainbow generator
}


void rainbowWithGlitter() 
{
  rainbow();      // built-in FastLED rainbow
  addGlitter(80); // plus some random sparkly glitter
}



void confetti() 
{
  fadeToBlackBy( leds, NUM_LEDS, 10);                    // random colored speckles that blink in and fade smoothly
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  fadeToBlackBy( leds, NUM_LEDS, 20);        // a colored dot sweeping back and forth, with fading trails
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}


void bpm()
{
  uint8_t BeatsPerMinute = 62;                      // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++)
  {  
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}


void juggle()
{
  fadeToBlackBy( leds, NUM_LEDS, 20);       // eight colored dots, weaving in and out of sync with each other
  byte dothue = 0;
  for( int i = 0; i < 8; i++ ) 
  {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV( dothue, 200, 255 );
    dothue += 32;
  }
}


void cyclonfadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

void cyclon()
{
  for(int i = 0; i < NUM_LEDS; i++) 
  {
    leds[i] = CHSV(hue++, 255, 255);
    FastLED.show(); //leds[i] = CRGB::Black;
    cyclonfadeall();
    delay(5);
  }

  for(int i = (NUM_LEDS)-1; i >= 0; i--) 
  {
    leds[i] = CHSV(hue++, 255, 255);
    FastLED.show();
    cyclonfadeall();
    delay(5);
  }  
}



uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }

  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  
  return i;
}


// Once you've gotten the basics working (AND NOT UNTIL THEN!)
// here's a helpful technique that can be tricky to set up, but 
// then helps you avoid the needs for sprinkling array-bound-checking
// throughout your code.
//
// It requires a careful attention to get it set up correctly, but
// can potentially make your code smaller and faster.
//
// Suppose you have an 8 x 5 matrix of 40 LEDs.  Normally, you'd
// delcare your leds array like this:
//    CRGB leds[40];
// But instead of that, declare an LED buffer with one extra pixel in
// it, "leds_plus_safety_pixel".  Then declare "leds" as a pointer to
// that array, but starting with the 2nd element (id=1) of that array: 
//    CRGB leds_with_safety_pixel[41];
//    CRGB* const leds( leds_plus_safety_pixel + 1);
// Then you use the "leds" array as you normally would.
// Now "leds[0..N]" are aliases for "leds_plus_safety_pixel[1..(N+1)]",
// AND leds[-1] is now a legitimate and safe alias for leds_plus_safety_pixel[0].
// leds_plus_safety_pixel[0] aka leds[-1] is now your "safety pixel".
//
// Now instead of using the XY function above, use the one below, "XYsafe".
//
// If the X and Y values are 'in bounds', this function will return an index
// into the visible led array, same as "XY" does.
// HOWEVER -- and this is the trick -- if the X or Y values
// are out of bounds, this function will return an index of -1.
// And since leds[-1] is actually just an alias for leds_plus_safety_pixel[0],
// it's a totally safe and legal place to access.  And since the 'safety pixel'
// falls 'outside' the visible part of the LED array, anything you write 
// there is hidden from view automatically.
// Thus, this line of code is totally safe, regardless of the actual size of
// your matrix:
//    leds[ XYsafe( random8(), random8() ) ] = CHSV( random8(), 255, 255);
//
// The only catch here is that while this makes it safe to read from and
// write to 'any pixel', there's really only ONE 'safety pixel'.  No matter
// what out-of-bounds coordinates you write to, you'll really be writing to
// that one safety pixel.  And if you try to READ from the safety pixel,
// you'll read whatever was written there last, reglardless of what coordinates
// were supplied.

CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
//CRGB* const leds( leds_plus_safety_pixel + 1);

uint16_t XYsafe( uint8_t x, uint8_t y)
{
  if( x >= kMatrixWidth) return -1;
  if( y >= kMatrixHeight) return -1;
  return XY(x,y);
}



void farbflash()
{
    uint32_t ms = millis();
    int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / kMatrixWidth));
    int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / kMatrixHeight));
    DrawOneFrame( ms / 65536, yHueDelta32 / 32768, xHueDelta32 / 32768);
    if( ms < 5000 ) {
      FastLED.setBrightness( scale8( BRIGHTNESS, (ms * 256) / 5000));
    } else {
      FastLED.setBrightness(BRIGHTNESS);
    }
    FastLED.show();
}

void DrawOneFrame( byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8)
{
  byte lineStartHue = startHue8;
  for( byte y = 0; y < kMatrixHeight; y++) {
    lineStartHue += yHueDelta8;
    byte pixelHue = lineStartHue;      
    for( byte x = 0; x < kMatrixWidth; x++) {
      pixelHue += xHueDelta8;
      leds[ XY(x, y)]  = CHSV( pixelHue, 255, 255);
    }
  }
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) 
  {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void isr_button1()
{
  detachInterrupt(digitalPinToInterrupt(BUTTON1_PIN));

  if ( ++state_1 > 8 ) { state_1 = 1; }

  state_1_new = 1;

}
