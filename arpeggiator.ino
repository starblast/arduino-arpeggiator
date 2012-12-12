#include <MIDI.h>


#define STAT1    7
#define STAT2    6
#define BUTTON1  2
#define BUTTON2  3
#define BUTTON3  4

#define UP      0;
#define DOWN    1;
#define UPDOWN  2;
#define MODES   3;


byte notes[10];
int tempo;
unsigned long time;
unsigned long blinkTime;
int playBeat;
int notesHeld;
int mode;
boolean blinkOn;
boolean hold;
boolean buttonOneDown;
boolean buttonTwoDown;
boolean buttonThreeDown;
boolean bypass;
boolean midiThruOn;

void setup() {
  blinkTime = time = millis();
  notesHeld = 0;
  playBeat=0;
  blinkOn = false;
  hold=true;
  buttonOneDown = buttonTwoDown = buttonThreeDown = false;
  mode=0;
  bypass = midiThruOn = false;

  pinMode(STAT1,OUTPUT);   
  pinMode(STAT2,OUTPUT);
  pinMode(BUTTON1,INPUT);
  pinMode(BUTTON2,INPUT);
  pinMode(BUTTON3,INPUT);

  digitalWrite(BUTTON1,HIGH);
  digitalWrite(BUTTON2,HIGH);
  digitalWrite(BUTTON3,HIGH);

  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);    

  // Connect the HandleNoteOn function to the library, so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
  MIDI.setHandleControlChange (HandleControlChange);
  MIDI.turnThruOff();

  digitalWrite(STAT1,HIGH);
  digitalWrite(STAT2,HIGH);

}


// This function will be automatically called when a NoteOn is received.
// It must be a void-returning function with the correct parameters,
// see documentation here: 
// http://arduinomidilib.sourceforge.net/class_m_i_d_i___class.html

void HandleNoteOn(byte channel, byte pitch, byte velocity) { 

  // Do whatever you want when you receive a Note On.

  if (velocity == 0)
    notesHeld--;
  else {
    if (notesHeld==0 && hold) 
      resetNotes();
    notesHeld++;
  }
  if (notesHeld > 0)
    digitalWrite(STAT2,LOW);
  else
    digitalWrite(STAT2,HIGH);


  for (int i = 0; i < sizeof(notes); i++) {

    if (velocity == 0) {
      if (!hold && notes[i] >= pitch) {
        // remove this note
        if (i < sizeof(notes))
          notes[i] = notes[i+1];
      }
    }
    else {
      if (notes[i] == '\0') {
        notes[i] = pitch;
        return;
      }

      if (notes[i] <= pitch)
        continue;

      for (int j = sizeof(notes); j > i; j--)
        notes[j] = notes[j-1];

      notes[i] = pitch;
      return;
    }

  }

  // Try to keep your callbacks short (no delays ect) as the contrary would slow down the loop()
  // and have a bad impact on real-time performance.
}


void HandleControlChange (byte channel, byte number, byte value) {
  MIDI.sendControlChange(number,value,channel); 
}

void loop() {

  // Call MIDI.read the fastest you can for real-time performance.
  MIDI.read();
//  
//  if (buttonOneDown && buttonThreeDown)
//    bypass = !bypass;
//    
//  if (bypass) {
//    if (!midiThruOn) {
//      MIDI.turnThruOn();
//      digitalWrite(STAT1,HIGH);
//    }
//    return;
//  }
//  else
//    if (midiThruOn) {
//      MIDI.turnThruOff();
//      digitalWrite(STAT1,LOW);
//    }


  
  unsigned int tick = millis();
  boolean buttonOnePressed = button(BUTTON1);
  boolean buttonTwoPressed = button(BUTTON2);
  boolean buttonThreePressed = button(BUTTON3);


  if (buttonOnePressed) {
    if (!buttonOneDown) {
      hold = !hold;
      buttonOneDown = true;
      resetNotes();
    }
  }
  else
    buttonOneDown = false;

  if (buttonTwoPressed) {
    if (!buttonTwoDown) {
      buttonTwoDown = true;
      playBeat=0;
      mode++;
      if (mode == 2) {
        mode=0;
      }
    }
  }
  else
    buttonTwoDown = false;


  if (buttonThreePressed) {
    buttonThreeDown = true;
    resetNotes();
  }
  else
    buttonThreeDown = false;





  tempo = (analogRead(1)/8)*10 + 80;

  if (tick - time > tempo) {

    if (blinkOn) {
      digitalWrite(STAT1,LOW);
      blinkOn = false;
    }
    else {
      digitalWrite(STAT1,HIGH);
      blinkOn = true; 
    }
    // stop the previous note
    MIDI.sendNoteOff(notes[playBeat],0,1);

    if ((hold || notesHeld > 0) && notes[0] != '\0') { 
      time = tick;
      
      if (mode == 0) {
        playBeat++;
        if (notes[playBeat] == '\0')
          playBeat=0;        
      }
      else if (mode == 1) {
        if (playBeat == 0) {
          playBeat = sizeof(notes)-1;
          while (notes[playBeat] == '\0') {
            playBeat--;
          }
        }        
        else       
          playBeat--;
      }
      

      // trigger the current note
      int velocity = 127 - analogRead(0)/8;
      if (velocity == 0)
        velocity++;
      MIDI.sendNoteOn(notes[playBeat],velocity,1);

    }
  }
}

void resetNotes() {
  for (int i = 0; i < sizeof(notes); i++)
    notes[i] = '\0';
}

char button(char button_num)
{
  return (!(digitalRead(button_num)));
}

