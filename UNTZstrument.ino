#include <Wire.h>
#include <Adafruit_Trellis.h>

#define LED     13 // Pin for heartbeat LED (shows code is working)
#define CHANNEL 1  // MIDI channel number

Adafruit_Trellis trellis;

uint8_t       heart        = 0;  // Heartbeat LED counter
unsigned long prevReadTime = 0L; // Keypad polling timer
uint8_t       gp1;
uint8_t       gp2;
uint8_t       gp3;
uint8_t       gp4;  
uint8_t       gp1_ctrlcode = 16;
uint8_t       gp2_ctrlcode = 17;
uint8_t       gp3_ctrlcode = 18;
uint8_t       gp4_ctrlcode = 19;  

// 48 being C2
uint8_t note[] = {
  60, 61, 62, 63,
  56, 57, 58, 59,
  52, 53, 54, 55,
  48, 49, 50, 51
};

uint8_t spiral[] = {
   0,  4,  8, 12,
  13, 14, 15, 11,
   7,  3,  2,  1,
   5,  9,  10, 6
};

void setup() {
  pinMode(LED, OUTPUT);
  trellis.begin(0x70); // Pass I2C address
#ifdef __AVR__
  // Default Arduino I2C speed is 100 KHz, but the HT16K33 supports
  // 400 KHz.  We can force this for faster read & refresh, but may
  // break compatibility with other I2C devices...so be prepared to
  // comment this out, or save & restore value as needed.
  TWBR = 12;
#endif

  // Little start up visual candy
  // light em up
  trellis.clear();
  for (uint8_t i=0; i<16; i++) {
    trellis.setLED(spiral[i]);
    trellis.writeDisplay();    
    delay(50);
  }
  // then turn them off
  for (uint8_t i=0; i<16; i++) {
    trellis.clrLED(spiral[i]);
    trellis.writeDisplay();    
    delay(50);
  }
  
  trellis.clear();
  trellis.writeDisplay();
  gp1 = map(analogRead(3), 0, 1023, 0, 127);
  gp2 = map(analogRead(2), 0, 1023, 0, 127);
  gp3 = map(analogRead(1), 0, 1023, 0, 127);
  gp4 = map(analogRead(0),0, 1023, 0, 127);
  usbMIDI.sendControlChange(gp1_ctrlcode, gp1, CHANNEL);
  usbMIDI.sendControlChange(gp2_ctrlcode, gp2, CHANNEL);
  usbMIDI.sendControlChange(gp3_ctrlcode, gp3, CHANNEL);
  usbMIDI.sendControlChange(gp4_ctrlcode, gp4, CHANNEL);
}

void loop() {
  unsigned long t = millis();
  if((t - prevReadTime) >= 20L) { // 20ms = min Trellis poll time
    if(trellis.readSwitches()) {  // Button state change?
      
      for(uint8_t i=0; i<16; i++) { // For each button...
        if(trellis.justPressed(i)) {
          usbMIDI.sendNoteOn(note[i], 127, CHANNEL);
          trellis.setLED(i);
        } else if(trellis.justReleased(i)) {
          usbMIDI.sendNoteOff(note[i], 0, CHANNEL);
          trellis.clrLED(i);
        }
      }
      trellis.writeDisplay();
    }
    uint8_t generalPurpose1 = map(analogRead(3), 0, 1023, 0, 127);
    if(gp1 != generalPurpose1) {
      gp1 = generalPurpose1;
      usbMIDI.sendControlChange(gp1_ctrlcode, gp1, CHANNEL);
    }
    uint8_t generalPurpose2 = map(analogRead(2), 0, 1023, 0, 127);
    if(gp2 != generalPurpose2) {
      gp2 = generalPurpose2;
      usbMIDI.sendControlChange(gp2_ctrlcode, gp2, CHANNEL);
    }
    uint8_t generalPurpose3 = map(analogRead(1), 0, 1023, 0, 127);
    if(gp3 != generalPurpose3) {
      gp3 = generalPurpose3;
      usbMIDI.sendControlChange(gp3_ctrlcode, gp3, CHANNEL);
    }
    uint8_t generalPurpose4 = map(analogRead(0), 0, 1023, 0, 127);
    if(gp4 != generalPurpose4) {
      gp4 = generalPurpose4;
      usbMIDI.sendControlChange(gp4_ctrlcode, gp4, CHANNEL);
    }
    prevReadTime = t;
    digitalWrite(LED, ++heart & 32); // Blink = alive
  }
  while(usbMIDI.read()); // Discard incoming MIDI messages
}
