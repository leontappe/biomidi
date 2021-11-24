#include <Arduino.h>
#include <BLEMidi.h>

#define SIGNAL_IN 4
#define LED_PIN 2

#define BT_ENABLED 1
#define LED_ENABLED 1

#define METER (double)8
#define BPM 174
#define NOTE_DURATION_MS (int)(((1000.0 / (BPM / 60.0)) * (4.0 / METER)))
#define NUM_OF_SAMPLES 4

#define TIMEOUT_STEPS 1
#define TIMEOUT_MS (TIMEOUT_STEPS * NOTE_DURATION_MS)

#define MIN_MIDI_NOTE 32
#define MAX_MIDI_RANGE 64

int last_millis = millis();
int last_time_played = last_millis;

int midi_value = MIN_MIDI_NOTE;
int last_midi_value = MIN_MIDI_NOTE;

int last_values[NUM_OF_SAMPLES];
int last_value_index = 0;
bool note_is_on = false;

bool led_on = false;

void IRAM_ATTR isr()
{
#ifdef LED_ENABLED
  led_on = !led_on;
  digitalWrite(LED_PIN, led_on);
#endif

  int current = millis();
  int diff = current - last_millis;
  last_millis = current;
  last_values[last_value_index++] = diff;
  if (last_value_index == NUM_OF_SAMPLES)
    last_value_index = 0;
}

int meanOfDiffs()
{
  int sum = 0;
  for (int i = 0; i < NUM_OF_SAMPLES; i++)
  {
    sum += last_values[i];
  }
  return sum / NUM_OF_SAMPLES;
}

int getMidiValue()
{
  int mean = meanOfDiffs();
  if (mean == 0 || mean == 1)
    return -1;
  double frequency = 1000.0 / mean;

  int result = (frequency / 100.0) * MAX_MIDI_RANGE + MIN_MIDI_NOTE;

  if (result > MIN_MIDI_NOTE + MAX_MIDI_RANGE)
  {
    midi_value = MIN_MIDI_NOTE + MAX_MIDI_RANGE;
  }
  else
  {
    midi_value = result;
  }

  return 0;
}

void playNote()
{
  if (!note_is_on)
  {
#ifdef BT_ENABLED
    BLEMidiServer.noteOn(0, midi_value, 127);

    BLEMidiServer.controlChange(0, 1, midi_value);
#endif

    last_midi_value = midi_value;
    note_is_on = true;
  }
}

void stopNote()
{
  if (note_is_on)
  {
#ifdef BT_ENABLED
    BLEMidiServer.noteOff(0, last_midi_value, 127);
#endif
    note_is_on = false;
  }
}

void setup()
{
  Serial.begin(115200);

  // init IO
  pinMode(SIGNAL_IN, INPUT);
#ifdef LED_ENABLED
  pinMode(LED_PIN, OUTPUT);
#endif
  attachInterrupt(SIGNAL_IN, isr, CHANGE);

#ifdef BT_ENABLED
  Serial.println("Initializing bluetooth");
  BLEMidiServer.begin("BioMIDI");
  Serial.println("Waiting for connections...");
#endif
  //BLEMidiServer.enableDebugging();
}

void loop()
{
#ifdef BT_ENABLED
  if (BLEMidiServer.isConnected())
  {
#endif
    if (millis() - last_time_played > NOTE_DURATION_MS)
    {
      if (getMidiValue() != -1)
      {
        stopNote();
        if (millis() - last_millis < TIMEOUT_MS)
        {
          last_time_played = millis();
          Serial.printf("playing midi note %d\n", midi_value);
          playNote();
        }
      }
    }
    delay(10);
#ifdef BT_ENABLED
  }
#endif
}
