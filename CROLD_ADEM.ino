#include <Arduino.h>
#include "MIDIUSB.h"

#define INPUTPIN 10
#define LEDOUT 9

#define BREATHE 1 //comment out to deactivate
unsigned long breatheDelay = 10; //less is faster


void setup()
{
  Serial.begin(115200);
  // put your setup code here, to run once:
  pinMode(INPUTPIN, INPUT_PULLUP);
  pinMode(LEDOUT, OUTPUT);
}

uint8_t PushButton();
void noteOn(byte channel, byte pitch, byte velocity);
void noteOff(byte channel, byte pitch, byte velocity);

uint8_t prevBstate;

void loop()
{
  uint8_t Bstate = PushButton();
  if (Bstate != prevBstate)
  {
    prevBstate = Bstate;
    if (Bstate == 1)
    {
      noteOn(0, 48, 10); // Channel 0, middle C, normal velocity
      MidiUSB.flush();
      Serial.println("pushed");
    }
    else
    {
      noteOff(0, 48, 10); // Channel 0, middle C, normal velocity
      MidiUSB.flush();
      Serial.println("released");
    }
  }
}

uint8_t pushstate;
uint8_t prevPushstate;
unsigned long pushtimout = 0;

uint8_t PushButton()
{
  int state = digitalRead(INPUTPIN);

  if (prevPushstate != state)
  {
    if (millis() - pushtimout > 100)
    {
      prevPushstate = state;
      pushtimout = millis();
    }
    if (prevPushstate == 0)  
    {

#ifdef BREATHE
      analogWrite(LEDOUT, 0);
#elif
      digitalWrite(LEDOUT, 0);
#endif
    }
#ifndef BREATHE
    else 
    {
      digitalWrite(LED, HIGH);
    }
#endif
  }
  
#ifdef BREATHE
  if (prevPushstate == 1)
  {
    breathe();
  }
#endif

  return prevPushstate;
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity)
{
  midiEventPacket_t noteOn = \{0x09, 0x90 | channel, pitch, velocity\};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity)
{
  midiEventPacket_t noteOff = \{0x08, 0x80 | channel, pitch, velocity\};
  MidiUSB.sendMIDI(noteOff);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value)
{
  midiEventPacket_t event = \{0x0B, 0xB0 | channel, control, value\};
  MidiUSB.sendMIDI(event);
}

int brightness = 0;
uint8_t isUp = 0;
unsigned long breatheTimer = 0;
int fadeAmount = 1;

void breathe()
{
  if (millis() - breatheTimer > breatheDelay)
  {
    breatheTimer = millis();

    brightness = brightness + fadeAmount;
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
  }
  analogWrite(LEDOUT, brightness);
}
}
