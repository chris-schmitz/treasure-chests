#include <Adafruit_NeoPixel.h>
#include <Adafruit_CircuitPlayground.h>
#include <Servo.h>

#define STRIP_PIN 10
#define TRIGGER_PIN 0

Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, STRIP_PIN, NEO_GRB + NEO_KHZ800);
Servo servo;

bool trigger = false;

void setup()
{
    Serial.begin(115200);
    CircuitPlayground.begin();

    strip.begin();
    strip.setBrightness(75);
    strip.show();

    servo.attach(3);
    servo.write(0);

    pinMode(TRIGGER_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), triggerAction, RISING);
}

void triggerAction()
{
    Serial.println("interrupt triggered");
    trigger = true;
}

void loop()
{
    Serial.println("===triggerstate===");
    Serial.println(trigger);
    Serial.println("====");
    if (trigger == true)
    {
        Serial.println("Triggered");
        int pause = 1000;

        if (trigger == 1)
        {
            colorWipe(strip.Color(255, 0, 0), 15);
            servo.write(180);
            delay(pause);
            colorWipe(strip.Color(0, 0, 0), 15);
            servo.write(0);
            delay(pause);
        }
        trigger = false;
    }
    delay(1);
}

void colorWipe(uint32_t color, uint8_t wait)
{
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
        strip.setPixelColor(i, color);
        strip.show();
        delay(wait);
    }
}