#include <VL53L0X.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define USE_IR true
// this constant won't change.  It's the pin number
// of the sensor's output:
const int pingPin = 7;
const int pongPin = 8;
long nb_cycle_seen = 0;//iterations
const long consecutive_seen_threshold = 10;//iterations
const int distance_seen_threshold = 200;//mm
VL53L0X sensor;

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_LED 6 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 16 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

void vitrine_on() {
  pixels.clear(); // Set all pixel colors to 'off'

  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...

    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));

    pixels.show();   // Send the updated pixel colors to the hardware.

    delay(DELAYVAL); // Pause before next pass through loop
  }
}

void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  Wire.begin();
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'

#ifdef USE_IR
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {}
  }
  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  sensor.startContinuous(100);
#endif
}

void loop()
{
  // establish variables for duration of the ping, 
  // and the distance result in inches and centimeters:
  long duration, distance_mm;

#ifdef USE_ULTRASOUND
  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pongPin, INPUT);
  duration = pulseIn(pongPin, HIGH);

  // convert the time into a distance
  distance_mm = microsecondsToCentimeters(duration) * 10; 
#endif
#ifdef USE_IR
  distance_mm = sensor.readRangeContinuousMillimeters();
  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
#endif
  Serial.print(distance_mm);
  Serial.print("mm");
  Serial.println();
  
  delay(100);

  if (distance_mm < distance_seen_threshold) {
    nb_cycle_seen++;
  }
  else {
    nb_cycle_seen = 0;
  }
  if (nb_cycle_seen > consecutive_seen_threshold)
  {
   vitrine_on(); 
  }
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
