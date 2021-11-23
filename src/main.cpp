#include <Arduino.h>
#include <BLEMidi.h>

#define SIGNAL_IN 4
#define LED 2

int count = 0;
int highest = 127;
int last_note = 0;

bool led_on = false;

void IRAM_ATTR isr()
{
  count += 1;
  led_on = !led_on;
  digitalWrite(LED, led_on);
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
  // reset freq after 1 second
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 1000)
  {
    lastMillis = millis();
    if (count > highest)
    {
      highest = count;
    }
    if (count > 0 && BLEMidiServer.isConnected())
    {
      BLEMidiServer.noteOff(0, last_note, 127);

      Serial.printf("receiving %d Hz\n", count);
      uint8_t midi_note = abs((double)count * 127.0 / (double)highest);

      Serial.printf("playing midi note %d\n", midi_note);
      BLEMidiServer.noteOn(0, midi_note, 127);

      BLEMidiServer.controlChange(0, 1, midi_note);

      last_note = midi_note;
    }
    // reset counter
    count = 0;
  }
}
