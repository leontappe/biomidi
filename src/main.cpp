#include <Arduino.h>
#include <BLEMidi.h>

#define SIGNAL_IN 4
#define LED 2

int last_millis = millis();

int midi_value = 0;
int last_value = 0;

int max_frequency = 127;

bool led_on = false;

void IRAM_ATTR isr()
{
  int current = millis();
  int diff = current - last_millis;
  last_millis = current;
  if (diff > 1)
  {
    double frequency = 1000.0 / diff;
    midi_value = frequency * 127 / max_frequency;
    if (midi_value > 127)
    {
      midi_value = 127;
    }
    if (frequency > max_frequency)
    {
      max_frequency = frequency;
    }
    led_on = !led_on;
    digitalWrite(LED, led_on);
  }
}

void setup()
{
  Serial.begin(115200);

  // init IO
  pinMode(SIGNAL_IN, INPUT);
  pinMode(LED, OUTPUT);
  attachInterrupt(SIGNAL_IN, isr, CHANGE);

  Serial.println("Initializing bluetooth");
  BLEMidiServer.begin("BioMIDI");
  Serial.println("Waiting for connections...");
  //BLEMidiServer.enableDebugging();
}

void loop()
{
  if (BLEMidiServer.isConnected())
  {
    BLEMidiServer.noteOff(0, last_value, 127);
    if (millis() - last_millis < 500)
    {
      Serial.printf("playing midi note %d\n", midi_value);
      BLEMidiServer.noteOn(0, midi_value, 127);

      BLEMidiServer.controlChange(0, 1, midi_value);

      last_value = midi_value;
    }
  }
  delay(50);
}
