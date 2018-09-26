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
int triggerDistance = 20;

uint8_t volumeLevel = 10;
uint8_t newVolumeLevel = volumeLevel;
uint8_t VOLUME_CHANGE_STEP = 5;

// ! Something important to note about how the music maker library handles volume is that
// ! setting the volume with lower numbers makes the volume louder and vice versa. So here
// ! when we're referring to the `VOLUME_LOWER_LIMIT` and `VOLUME_UPPER_LIMIT`, we're referring
// ! to the number values passed in, not the loudness level we want the speaker to be at.
// ! (hopefully that makes sense :| )
uint8_t VOLUME_LOWER_LIMIT = 0;  // * this is the cap on how loud we get
uint8_t VOLUME_UPPER_LIMIT = 50; // * this is the cap on how soft we get

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
    pinMode(A2, OUTPUT);

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

    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT); // DREQ int

    // Set volume for left, right channels. lower numbers == louder volume!
    musicPlayer.setVolume(volumeLevel, volumeLevel);

    attachInterrupt(digitalPinToInterrupt(VOLUME_UP), volumeUp, RISING);
    attachInterrupt(digitalPinToInterrupt(VOLUME_DOWN), volumeDown, RISING);
}

void loop()
{

    strip.show();
    delay(100);
    // // ! Note that I'm putting the up and down check into an if/elseif structure
    // // ! with down taking higher importance.
    // // ! This way if the user holds down both buttons, the default is for the
    // // ! volume to go down, not up.
    checkVolume();

    distance = checkDistance();

    if (distance > triggerDistance)
    {
        return; // ! skip, nothing to do
    }

    Serial.println("within trigger distance!");
    musicPlayer.stopPlaying();
    playSlideWhistle();
}

long checkDistance()
{
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
    return distance;
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
    if (musicPlayer.playingMusic)
    {
        Serial.println("music is currently playing, can't slide whistle again");
        return;
    }

    musicPlayer.startPlayingFile("track003.mp3");
    colorWipe(strip.Color(255, 0, 0), 17);
    delay(5);
    servoWave();
    colorWipe(strip.Color(0, 0, 0), 17, 1);
}

void servoWave()
{
    Serial.println("waving");
    uint8_t startPosition = 0;
    uint8_t endPosition = 180;
    uint8_t pause = 6;
    myservo.write(0);

    for (int i = startPosition; i <= endPosition; i++)
    {
        myservo.write(i);
        delay(pause);
    }

    for (int i = endPosition; i > startPosition; i--)
    {
        myservo.write(i);
        delay(pause);
    }
    myservo.write(0);
}

/**
 * * The volumeUp and volumeDown functions are Interrupt Service Routines or ISRs
 * * For more information on ISRs and why I'm using them here, check out:
 * ? https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
 */

void volumeUp()
{
    // Serial.println("++++++++++++++++++++++++++++++++++++");
    // Serial.print("current volume level: ");
    // Serial.println(volumeLevel);
    // Serial.print("volume level minus step: ");
    // Serial.println(volumeLevel - VOLUME_CHANGE_STEP);
    // Serial.print("volume level minus step is greater than volume level: ");
    // Serial.println((volumeLevel - VOLUME_CHANGE_STEP < VOLUME_LOWER_LIMIT));
    // Serial.println("++++++++++++++++++++++++++++++++++++");
    if ((volumeLevel - VOLUME_CHANGE_STEP < VOLUME_LOWER_LIMIT))
    {
        newVolumeLevel = VOLUME_LOWER_LIMIT;
        return;
    }
    newVolumeLevel = volumeLevel - VOLUME_CHANGE_STEP;
}
void volumeDown()
{
    // Serial.println("++++++++++++++++++++++++++++++++++++");
    // Serial.print("current volume level: ");
    // Serial.println(volumeLevel);
    // Serial.print("volume level plus step: ");
    // Serial.println(volumeLevel + VOLUME_CHANGE_STEP);
    // Serial.print("volume level minus step is greater than volume level: ");
    // Serial.println((volumeLevel + VOLUME_CHANGE_STEP > VOLUME_LOWER_LIMIT));
    // Serial.println("++++++++++++++++++++++++++++++++++++");
    if ((volumeLevel + VOLUME_CHANGE_STEP > VOLUME_UPPER_LIMIT))
    {
        newVolumeLevel = VOLUME_UPPER_LIMIT;
        return;
    }
    newVolumeLevel = volumeLevel + VOLUME_CHANGE_STEP;
}

void checkVolume()
{
    if (volumeLevel != newVolumeLevel)
    {
        volumeLevel = newVolumeLevel;
        adjustVolume(volumeLevel);
    }
}

void adjustVolume(int level)
{
    // * These checks are kind of unnecessary in the grand scheme of this particular sketch.
    // * In the `volumeUp` and `volumeDown` interrupt functions we're checking to make sure the
    // * new volume that we're we're setting isn't outside of our determined range.
    // * That said, this `adjustVolume` function could possibly be called from anywhere and have
    // * any integer value passed in, so it's worth doing a quick check here to make sure we're not
    // * out of range as well.
    bool aboveLowerLimit = (level <= VOLUME_UPPER_LIMIT);
    bool belowUpperLimit = (level >= VOLUME_LOWER_LIMIT);
    bool inRange = belowUpperLimit && aboveLowerLimit;

    // Serial.println("=====================================");
    // Serial.print("Lower Limit Value: ");
    // Serial.println(VOLUME_LOWER_LIMIT);
    // Serial.print("Upper Limit Value: ");
    // Serial.println(VOLUME_UPPER_LIMIT);
    // Serial.print("New level value: ");
    // Serial.println(level);
    // Serial.print("Current volume level: ");
    // Serial.println(volumeLevel);
    // Serial.println("-------------------------------------");

    // Serial.print("level is above lower limit: ");
    // Serial.println(aboveLowerLimit);
    // Serial.print("level is below upper limit: ");
    // Serial.println(belowUpperLimit);
    // Serial.print("level is within range: ");
    // Serial.println(inRange);
    // Serial.print("the new volume level is: ");
    // Serial.println(level);
    // Serial.println("=====================================");

    if (inRange == true)
    {
        musicPlayer.setVolume(level, level);
        return;
    }
}