/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h>

#define triggerPin 3
#define echoPin 4
const int servoPin = 2;

Servo myservo; // create servo object to control a servo

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

int pos = 0; // variable to store the servo position
long duration;
long distance;

void setup()
{
    Serial.begin(9600);
    // * Attach servo to pin
    myservo.attach(servoPin);

    // * Setup ultrasonic sensor
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

void loop()
{
    // for (pos = 0; pos <= 180; pos += 1)
    // { // goes from 0 degrees to 180 degrees
    //     // in steps of 1 degree
    //     myservo.write(pos); // tell servo to go to position in variable 'pos'
    //     delay(15);          // waits 15ms for the servo to reach the position
    // }
    // for (pos = 180; pos >= 0; pos -= 1)
    // {                       // goes from 180 degrees to 0 degrees
    //     myservo.write(pos); // tell servo to go to position in variable 'pos'
    //     delay(15);          // waits 15ms for the servo to reach the position
    // }
    // return;

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

    Serial.print("Duration: ");
    Serial.println(duration);
    Serial.print("Distance: ");
    Serial.println(distance);

    if (distance > 50)
    {
        Serial.println("Writing 0");
        myservo.write(0);
    }
    else if (distance < 500)
    {
        Serial.println("Writing 180");
        myservo.write(180);
    }

    delay(50);

    // for (pos = 0; pos <= 180; pos += 1)
    // { // goes from 0 degrees to 180 degrees
    //     // in steps of 1 degree
    //     myservo.write(pos); // tell servo to go to position in variable 'pos'
    //     delay(15);          // waits 15ms for the servo to reach the position
    // }
    // for (pos = 180; pos >= 0; pos -= 1)
    // {                       // goes from 180 degrees to 0 degrees
    //     myservo.write(pos); // tell servo to go to position in variable 'pos'
    //     delay(15);          // waits 15ms for the servo to reach the position
    // }
}
