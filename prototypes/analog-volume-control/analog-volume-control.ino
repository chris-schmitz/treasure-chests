// #include <SD.h>
// #include <Adafruit_VS1053.h>
#include <Adafruit_NeoPixel.h>

// #define VS1053_RESET -1 // VS1053 reset pin (not used!)
// // * Feather M4 M0, 328, or 32u4
// #define VS1053_CS 6   // VS1053 chip select pin (output)
// #define VS1053_DCS 10 // VS1053 Data/command select pin (output)
// #define CARDCS 5      // Card chip select pin
// #define VS1053_DREQ 9 // VS1053 Data request, ideally an Interrupt pin

const int potPin = A0;
const int neoPixelPin = 12;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, neoPixelPin, NEO_GRB + NEO_KHZ800);

int sensorValue = 0;
int outputValue = 0;

void setup()
{
    Serial.begin(9600);
    strip.begin();
    strip.setPixelColor(0, 255, 255, 0);
    strip.show();
}

void loop()
{
    sensorValue = analogRead(potPin);
    outputValue = map(sensorValue, 0, 1023, 0, 255);
    uint16_t pixelCount = map(sensorValue, 0, 1023, 0, 30);

    Serial.println("==============");
    Serial.print("sensor value: ");
    Serial.println(sensorValue);
    Serial.print("output value: ");
    Serial.println(outputValue);
    Serial.print("pixel count: ");
    Serial.println(pixelCount);
    Serial.println("==============");

    lightPixels(pixelCount);

    delay(10);
}

void lightPixels(uint16_t count)
{
    // clearStrip();

    for (uint16_t i = 0; i <= strip.numPixels(); i++)
    {
        Serial.println("+++++");
        Serial.print("count: ");
        Serial.println(count);
        Serial.print("i: ");
        Serial.println(i);
        Serial.print("strip color for pixel i: ");
        Serial.println(strip.getPixelColor(i));
        Serial.print("i <= count: ");
        Serial.println((i <= count));
        Serial.println("+++++");
        if (i < count)
        {
            Serial.println("setting pixel color!");
            strip.setPixelColor(i, strip.Color(0, 255, 20));
        }
        else
        {
            Serial.println("clearing pixel color!");
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
