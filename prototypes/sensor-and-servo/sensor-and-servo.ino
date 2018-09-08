#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

// ! bluefruit nRF5 pins
// #define servoPin 2
// #define triggerPin 3
// #define echoPin 4
// #define neopixelPin 5
// #define speakerPin

// // ! adalogger M0 express pins
// #define servoPin 14
// #define triggerPin 13
// #define echoPin 16
// #define neopixelPin 17
// #define speakerPin 18

// ! feather huzzah
#define servoPin 15
#define triggerPin 14
#define echoPin 12
#define neopixelPin 13
#define speakerPin 16

Servo myservo;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, neopixelPin, NEO_GRB + NEO_KHZ800);

int pos = 0; // variable to store the servo position
long duration;
long distance;
bool triggerClose = false;

void setup()
{
    Serial.begin(9600);
    // * Attach servo to pin
    myservo.attach(servoPin);

    // * Setup ultrasonic sensor
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);

    // * Set up neopixel strip
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
}

void loop()
{

    // * Get back into a ready state for the ultrasonic sensor
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);

    // * Send out a pulse from the trigger
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    // * Look for a reply pulse from the echo pin
    duration = pulseIn(echoPin, HIGH);

    // * Calculate distance based on the speed of sound
    // ! note that the ` / 2` is because the pulse duration accounts for the time
    // ! the pulse is sent out and coming back, to calculate distance we only want
    // ! HALF of that round trip
    distance = duration * 0.034 / 2;

    Serial.println("=============");
    Serial.print("Duration: ");
    Serial.println(duration);
    Serial.print("Distance: ");
    Serial.println(distance);
    Serial.println("=============");
    return;

    if (distance < 50)
    {
        Serial.println("close");
        if (triggerClose == false)
        {
            Serial.println("set Trigger close true");
            triggerClose = true;
            myservo.write(0);
            tone(speakerPin, 1000, 500);
            colorWipe(strip.Color(255, 0, 0), 5);
        }
    }
    else if (distance >= 50)
    {
        Serial.println("far");
        if (triggerClose == true)
        {
            Serial.println("set Trigger close false");
            triggerClose = false;
            myservo.write(180);
            colorWipe(strip.Color(0, 0, 0), 5);
        }
    }

    delay(50);
}

void peek()
{
    // myservo.write
}

void halfOpen() {}

void open() {}

// ! Pulled from github issues comment: https://github.com/sandeepmistry/arduino-nRF5/issues/23#issuecomment-232268164
// ! Come back later and extract this to it's own file and try to understand what is going on here.
// See pulse_asm.S
//TODO extern unsigned long countPulseASM(const volatile uint32_t *port, uint32_t bit, uint32_t stateMask, unsigned long maxloops);
unsigned long countPulse_C(const volatile uint32_t *port, uint32_t bit, uint32_t stateMask, unsigned long maxloops)
{
    unsigned long width = 0;

    // wait for any previous pulse to end
    while ((*port & bit) == stateMask)
        if (--maxloops == 0)
            return 0;

    // wait for the pulse to start
    while ((*port & bit) != stateMask)
        if (--maxloops == 0)
            return 0;

    // wait for the pulse to stop
    while ((*port & bit) == stateMask)
    {
        if (++width == maxloops)
            return 0;
    }
    return width;
}

// ! From the adafruit strandtest
// * Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait)
{
    uint16_t i, j;

    for (j = 0; j < 256 * 5; j++)
    { // 5 cycles of all colors on wheel
        for (i = 0; i < strip.numPixels(); i++)
        {
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// * Pulled over from the `strandtest.ino` example
void colorWipe(uint32_t c, uint8_t wait)
{
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
}