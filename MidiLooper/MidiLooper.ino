#include <MIDI.h>

// MidiLooper - target: the 1010music Euroshield with the PJRC Teensy 3.2
// Created by Scott Metoyer, 2020
//
// Midi tape-style looping performance utility for the 1010music Euroshield.
//
// This code is in the public domain.

#define buttonPin             2
#define ledPin                13
#define maxNumberOfSequences  4
#define maxNumberOfEvents     32
#define debounceMS            25
#define ledPinCount           4
#define shortPress            40
#define longPress             1000

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

int recordingSequenceIndex = 0;
unsigned long lastUpdate;
int ledPins[ledPinCount] = { 3, 4, 5, 6 };
unsigned long lastButtonMS = 0;
int lastButtonState = -1;
unsigned long buttonCounter = 0;
bool isRecording = false;
int eventCounter = 0;

struct Note {
  byte pitch;
  byte velocity;
  long timestamp;
};

struct Sequence {
  Note note[maxNumberOfEvents];
  int currentEventIndex;
  int numberOfEvents;
  unsigned long start;
  unsigned long end;
  unsigned long elapsed;
};

Sequence sequence[maxNumberOfSequences];

void startRecording() {
  digitalWrite(ledPin, HIGH);
  isRecording = true;
}

void stopRecording() {
  digitalWrite(ledPin, LOW);
  isRecording = false;

  // Set the current sequence end time, number of events, and reset the event index
  sequence[recordingSequenceIndex].end = millis();
  sequence[recordingSequenceIndex].currentEventIndex = 0;
  sequence[recordingSequenceIndex].numberOfEvents = eventCounter;
  eventCounter = 0;

  // Increment the sequence index
  if (recordingSequenceIndex == maxNumberOfSequences - 1) {
    recordingSequenceIndex = 0;
  } else {
     recordingSequenceIndex++;
  }
}

void recordEvent(byte pitch, byte velocity)
{
  if (isRecording) {
    long ms = millis();

    // Start the sequence clock and clear the sequence if this is the first incoming note
    if (eventCounter == 0) {
      sequence[recordingSequenceIndex].start = ms;
      sequence[recordingSequenceIndex].elapsed = ms;
      memset(sequence[recordingSequenceIndex].note, 0, sizeof(sequence[recordingSequenceIndex].note));
    }

    sequence[recordingSequenceIndex].note[eventCounter].pitch = pitch;
    sequence[recordingSequenceIndex].note[eventCounter].velocity = velocity;
    sequence[recordingSequenceIndex].note[eventCounter].timestamp = ms;

    // If we have filled the note buffer then turn off recording
    eventCounter++;

    if (eventCounter == maxNumberOfEvents) {
      stopRecording();
    }
  }
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  digitalWrite(ledPins[recordingSequenceIndex], HIGH);
  recordEvent(pitch, velocity);
  MIDI.sendNoteOn(pitch, velocity, recordingSequenceIndex + 1);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  digitalWrite(ledPins[recordingSequenceIndex], LOW);
  recordEvent(pitch, 0);
  MIDI.sendNoteOff(pitch, velocity, recordingSequenceIndex + 1);
}

void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  for (int i = 0; i < ledPinCount; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);

  lastButtonState = digitalRead(buttonPin);
}

void loop()
{
  // Check record button
  int buttonState = digitalRead(buttonPin);

  if (buttonState != lastButtonState) {
      long ms = millis();

      if (ms - lastButtonMS > debounceMS) {
        if (buttonState == LOW) {
          // A button down event has occurred. Start the button counter.
          buttonCounter = ms;
        } else {
          // A button up even has occurred. Either toggle recording, or clear the sequences depending on how long the button was pressed.
          if ((ms - buttonCounter >= shortPress) && !(ms - buttonCounter >= longPress)) {
              // Short press detected
              if (isRecording) {
                stopRecording();
              } else {
                startRecording();
              }
          }
          if ((ms - buttonCounter >= longPress)) {
              // Long press detected
              memset(sequence, 0, sizeof(sequence));
              recordingSequenceIndex = 0;
          }
        }
      }

      lastButtonMS = ms;
      lastButtonState = buttonState;
  }

  // Check for incoming Midi messages. They are handled in the note callbacks.
  MIDI.read();

  // Play the sequences
  long ms = millis();
  long elapsed = ms - lastUpdate;

  for (int i = 0; i < maxNumberOfSequences; i++) {
    // Don't try to play the currently recording sequnce
    if (isRecording && i == recordingSequenceIndex) {
      break;
    }

    sequence[i].elapsed += elapsed;

    // Loop if we have reached the end of the sequence
    if (sequence[i].elapsed >= sequence[i].end) {
      sequence[i].elapsed = sequence[i].start;
      sequence[i].currentEventIndex = 0;
    }

    int eventIdx = sequence[i].currentEventIndex;

    // Play events as we reach them in the timeline
    if (eventIdx < sequence[i].numberOfEvents) {
      if (sequence[i].elapsed >= sequence[i].note[eventIdx].timestamp) {
        byte pitch = sequence[i].note[eventIdx].pitch;
        byte velocity = sequence[i].note[eventIdx].velocity;

        if (velocity > 0) {
          digitalWrite(ledPins[i], HIGH);
          MIDI.sendNoteOn(pitch, velocity, i + 1);
        } else {
          digitalWrite(ledPins[i], LOW);
          MIDI.sendNoteOff(pitch, velocity, i + 1);
        }

        sequence[i].currentEventIndex++;
      }
    }
  }

  lastUpdate = ms;
}
