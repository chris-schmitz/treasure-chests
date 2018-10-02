#include <SD.h>
#include <Adafruit_VS1053.h>
#include <Adafruit_NeoPixel.h>

#define VS1053_RESET -1 // VS1053 reset pin (not used!)
// * Feather M4 M0, 328, or 32u4
#define VS1053_CS 6   // VS1053 chip select pin (output)
#define VS1053_DCS 10 // VS1053 Data/command select pin (output)
#define CARDCS 5      // Card chip select pin
#define VS1053_DREQ 9 // VS1053 Data request, ideally an Interrupt pin

const int potPin = A0;
const int neoPixelPin = 12;

Adafruit_VS1053_FilePlayer soundPlayer = Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, neoPixelPin, NEO_GRB + NEO_KHZ800);

int sensorValue = 0;
int outputValue = 0;
const uint8_t DEFAULT_VOLUME = 20;

void setup()
{
    Serial.begin(9600);

    // * Setup for neopixel strip
    strip.begin();
    strip.setPixelColor(0, 255, 255, 0);
    strip.show();

    // * Setup for music maker wing
    initMusicMakerWing();
}

void loop()
{
    sensorValue = analogRead(potPin);
    uint8_t volumeLevel = map(sensorValue, 0, 1023, 50, 0);
    uint8_t pixelCount = map(sensorValue, 0, 1023, 0, 30);

    Serial.println("==============");
    Serial.print("sensor value: ");
    Serial.println(sensorValue);
    Serial.print("volumeLevel: ");
    Serial.println(volumeLevel);
    Serial.print("pixel count: ");
    Serial.println(pixelCount);
    Serial.println("==============");

    lightPixels(pixelCount);
    setVolume(volumeLevel);

    delay(10);
}

void lightPixels(uint16_t count)
{
    if (!soundPlayer.playingMusic)
    {
        playFile("track003.mp3");
    }

    for (uint16_t i = 0; i <= strip.numPixels(); i++)
    {
        if (i < count)
        {
            strip.setPixelColor(i, strip.Color(0, 255, 20));
        }
        else
        {
            strip.setPixelColor(i, strip.Color(0, 0, 0));
        }
    }

    strip.show();
}

void clearStrip()
{

    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
}

void initMusicMakerWing()
{

    if (!soundPlayer.begin())
    {
        Serial.println(F("Could not start the music maker wing"));
        while (1)
            ;
    }
    Serial.println("Music maker wing setup");

    if (!SD.begin(CARDCS))
    {
        Serial.println(F("SD Card failed or is not present"));
        while (1)
            ;
    }

    Serial.println("SD Card recognized");

    soundPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);
    setVolume(DEFAULT_VOLUME);
}

void setVolume(uint8_t level)
{
    soundPlayer.setVolume(level, level);
}

void playFile(const char *name)
{
    if (soundPlayer.playingMusic)
    {
        return; // * music is already playing
    }
    soundPlayer.startPlayingFile(name);
}