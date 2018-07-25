#include <Servo.h>
#include <Adafruit_NeoPixel.h>

#define servoPin 2
#define triggerPin 3
#define echoPin 4
#define neopixelPin 5

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

    // Serial.print("Duration: ");
    // Serial.println(duration);
    Serial.print("Distance: ");
    Serial.println(distance);

    if (distance < 50)
    {
        Serial.println("close");
        if (triggerClose == false)
        {
            Serial.println("set Trigger close true");
            triggerClose = true;
            myservo.write(0);
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

/* Measures the length (in microseconds) of a pulse on the pin; state is HIGH
 * or LOW, the type of pulse to measure.  Works on pulses from 2-3 microseconds
 * to 3 minutes in length, but must be called at least a few dozen microseconds
 * before the start of the pulse. */
uint32_t pulseIn(uint32_t pin, uint32_t state, uint32_t timeout)
{
    // cache the port and bit of the pin in order to speed up the
    // pulse width measuring loop and achieve finer resolution.  calling
    // digitalRead() instead yields much coarser resolution.
    // PinDescription p = g_APinDescription[pin];
    uint32_t bit = 1 << pin; //p.ulPin;
    uint32_t stateMask = state ? bit : 0;

    // convert the timeout from microseconds to a number of times through
    // the initial loop; it takes (roughly) 13 clock cycles per iteration.
    uint32_t maxloops = microsecondsToClockCycles(timeout) / 13;

    uint32_t width = countPulse_C(&(NRF_GPIO->IN), bit, stateMask, maxloops);
    //TODO  uint32_t width = countPulseASM(&(NRF_GPIO->IN), bit, stateMask, maxloops);

    // convert the reading to microseconds. The loop has been determined
    // to be 13 clock cycles long and have about 16 clocks between the edge
    // and the start of the loop. There will be some error introduced by
    // the interrupt handlers.
    if (width)
        return clockCyclesToMicroseconds(width * 13 + 16);
    else
        return 0;
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