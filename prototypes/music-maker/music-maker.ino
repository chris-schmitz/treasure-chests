// Specifically for use with the Adafruit Feather, the pins are pre-set here!

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>
#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#define VS1053_RESET -1 // VS1053 reset pin (not used!)
// * Feather M4 M0, 328, or 32u4
#define VS1053_CS 6   // VS1053 chip select pin (output)
#define VS1053_DCS 10 // VS1053 Data/command select pin (output)
#define CARDCS 5      // Card chip select pin
#define VS1053_DREQ 9 // VS1053 Data request, ideally an Interrupt pin
#define NEOPIXEL_PIN 13
#define TRIGGER 14
#define ECHO 15
#define VOLUME_UP 0
#define VOLUME_DOWN 1

// * setting up vars
Adafruit_VS1053_FilePlayer musicPlayer =
    Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Servo myservo;

long duration;
long distance;
int triggerDistance = 60;
int volumeLevel = 20;

void setup()
{
    Serial.begin(115200);

    strip.begin();
    strip.show();

    pinMode(TRIGGER, OUTPUT);
    pinMode(ECHO, INPUT);

    pinMode(VOLUME_DOWN, INPUT);
    pinMode(VOLUME_UP, INPUT);

    myservo.attach(A2);

    Serial.println("\n\nAdafruit VS1053 Feather Test");

    if (!musicPlayer.begin())
    { // initialise the music player
        Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
        while (1)
            ;
    }

    Serial.println(F("VS1053 found"));

    // musicPlayer.sineTest(0x44, 500); // Make a tone to indicate VS1053 is working

    if (!SD.begin(CARDCS))
    {
        Serial.println(F("SD failed, or not present"));
        while (1)
            ; // don't do anything more
    }
    Serial.println("SD OK!");

    // list files
    // printDirectory(SD.open("/"), 0);

    // Set volume for left, right channels. lower numbers == louder volume!
    musicPlayer.setVolume(volumeLevel, volumeLevel);

    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT); // DREQ int

    attachInterrupt(digitalPinToInterrupt(VOLUME_UP), volumeUp, RISING);
    attachInterrupt(digitalPinToInterrupt(VOLUME_DOWN), volumeDown, RISING);
}

void loop()
{
    delay(100);

    // // ! Note that I'm putting the up and down check into an if/elseif structure
    // // ! with down taking higher importance.
    // // ! This way if the user holds down both buttons, the default is for the
    // // ! volume to go down, not up.
    // if (digitalRead(VOLUME_DOWN))
    // {
    //     adjustVolume("down");
    // }
    // else if (digitalRead(VOLUME_UP))
    // {
    //     adjustVolume("up");
    // }

    // * Prep ultrasonic sensor
    digitalWrite(TRIGGER, LOW);
    delayMicroseconds(2);

    // * Send out pulse
    digitalWrite(TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER, LOW);

    // * Get the pulse duration
    duration = pulseIn(ECHO, HIGH);

    // * Calc the distance
    distance = duration * 0.034 / 2;

    // Serial.println("=======");
    // Serial.print("Duration: ");
    // Serial.println(duration);
    // Serial.print("Distance: ");
    // Serial.println(distance);
    // Serial.println("=======");

    if (distance > triggerDistance)
    {
        Serial.println("not close enough");
        return; // ! skip, nothing to do
    }

    musicPlayer.stopPlaying();
    playSlideWhistle();
    // delay(100);
    return;
}

/// File listing helper
void printDirectory(File dir, int numTabs)
{
    while (true)
    {

        File entry = dir.openNextFile();
        if (!entry)
        {
            // no more files
            //Serial.println("**nomorefiles**");
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++)
        {
            Serial.print('\t');
        }
        Serial.print(entry.name());
        if (entry.isDirectory())
        {
            Serial.println("/");
            printDirectory(entry, numTabs + 1);
        }
        else
        {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}

void colorWipe(uint32_t c, uint8_t pause, bool reverse = 0)
{
    if (reverse == 1)
    {
        // ! for (uint16_t i = strip.numPixels(); i >= 0; --i)
        // ? note that the above for signature will not work.
        // ? we're using an _unsigned_ integer. When we go past 0 here
        // ? we enter exponent territory. Good explanation:
        // * https://stackoverflow.com/a/17027607/1934903
        for (int i = strip.numPixels(); i >= 0; --i)
        {
            strip.setPixelColor(i, c);
            strip.show();
            delay(pause);
        }
    }
    else
    {
        for (uint16_t i = 0; i < strip.numPixels(); i++)
        {
            strip.setPixelColor(i, c);
            strip.show();
            delay(pause);
        }
    }
}

void playSlideWhistle()
{
    musicPlayer.startPlayingFile("track001.wav");
    myservo.write(0);
    colorWipe(strip.Color(255, 0, 0), 17);
    delay(100);
    colorWipe(strip.Color(0, 0, 0), 17, 1);
    myservo.write(180);
}

/**
 * * The volumeUp and volumeDown functions are Interrupt Service Routines or ISRs
 * * For more information on ISRs and why I'm using them here, check out:
 * ? https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
 */

void volumeUp()
{
    adjustVolume("up");
}
void volumeDown()
{
    adjustVolume("down");
}

void adjustVolume(String direction)
{
    if (direction == "up" && volumeLevel > 10)
    {
        volumeLevel--; // * lower numbers == louder
    }
    else if (direction == "down" && volumeLevel < 40)
    {
        volumeLevel++; // * higher numbers == softer
    }

    Serial.print("Setting volume to level: ");
    Serial.println(volumeLevel);
    musicPlayer.setVolume(volumeLevel, volumeLevel);
}