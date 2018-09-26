
#define TRIGGER_PIN 11

void setup()
{
    Serial.begin(9600);
    pinMode(TRIGGER_PIN, OUTPUT);
}

void loop()
{
    delay(6000);
    Serial.println("triggering");
    digitalWrite(TRIGGER_PIN, HIGH);
    delay(100);
    digitalWrite(TRIGGER_PIN, LOW);
}