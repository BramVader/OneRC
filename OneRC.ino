#include <IRremote.h>

// Connect photodiode to pin 11, IR-led to pin 3
// The built-in LED on pin 13 is used to indicate RC1A is being suppressed
int RECV_PIN = 11;
int MONITOR_PIN = 13;

// We have three devices, with three separate remote controls
//   RC1: Arcadyan HMB2260 Set-top box
//     RC1A: Unknown protocol, Manchester encoding on 56kHz
//     RC1B: RC5-protocol 38kHz for TV
//   RC2: Philips TV (RC6-protocol)
//   RC3: Speakerset (NEC-protocol)
// We want RC2 to control all three devices.

// RC1A codes
#define RC1A_TOGGLE 0x80
#define RC1A_ADDRESS 0x294000

#define RC1A_ONOFF 11
#define RC1A_REWIND 33
#define RC1A_PAUSE 27
#define RC1A_FF 36
#define RC1A_STOP 34
#define RC1A_REC 35
#define RC1A_RED 29
#define RC1A_GREEN 30
#define RC1A_YELLOW 31
#define RC1A_BLUE 32
#define RC1A_UP 12
#define RC1A_LEFT 14
#define RC1A_OK 15
#define RC1A_RIGHT 16
#define RC1A_DOWN 13
#define RC1A_MENU 45
#define RC1A_TV 20
#define RC1A_GUIDE 22
#define RC1A_RADIO 43
#define RC1A_RETURN 19
#define RC1A_PROGUP 25
#define RC1A_PROGDOWN 28
#define RC1A_1 1
#define RC1A_2 2
#define RC1A_3 3
#define RC1A_4 4
#define RC1A_5 5
#define RC1A_6 6
#define RC1A_7 7
#define RC1A_8 8
#define RC1A_9 9
#define RC1A_0 10
#define RC1A_TELETEXT 41

// RC1B-codes
// NOTE: Every new key press toggles the use of RC1B_TOGGLE, OR'red on the key code.
// Repeats are just the same codes as the key press.
#define RC1B_TOGGLE 0x800
#define RC1B_VOLUP 0x10
#define RC1B_VOLDOWN 0x11
#define RC1B_MUTE 0x0D
#define RC1B_ONOFF 0x0C
#define RC1B_SOURCE 0x38

// RC2-codes
// NOTE: Every new key press toggles the use of RC2_TOGGLE, OR'red on the key code.
// Repeats are just the same codes as the key press.
#define RC2_TOGGLE 0x10000
#define RC2_ONOFF 0x0C
#define RC2_STOP 0x31
#define RC2_PAUSE 0x30
#define RC2_REC 0x37
#define RC2_REWIND 0x2B
#define RC2_PLAY 0x2C
#define RC2_FF 0x28
#define RC2_SETTINGS 0xBF
#define RC2_SEARCH 0xB4
#define RC2_PICFORMAT 0xF5
#define RC2_GUIDE 0xCC
#define RC2_LIST 0xD2
#define RC2_SOURCE 0x38

#define RC2_SMARTTV 0xBE
#define RC2_HOME 0x54
#define RC2_EXIT 0x9F
#define RC2_RED 0x6D
#define RC2_GREEN 0x6E
#define RC2_YELLOW 0x6F
#define RC2_BLUE 0x70

#define RC2_INFO 0x0F
#define RC2_OPTIONS 0x40
#define RC2_RETURN 0x0A
#define RC2_MULTIVIEW 0x5D

#define RC2_UP 0x58
#define RC2_DOWN 0x59
#define RC2_LEFT 0x5A
#define RC2_RIGHT 0x5B
#define RC2_OK 0x5C

#define RC2_VOLUP 0x10
#define RC2_VOLDOWN 0x11
#define RC2_MUTE 0x0D
#define RC2_PROGUP 0x20
#define RC2_PROGDOWN 0x21

#define RC2_0 0x00
#define RC2_1 0x01
#define RC2_2 0x02
#define RC2_3 0x03
#define RC2_4 0x04
#define RC2_5 0x05
#define RC2_6 0x06
#define RC2_7 0x07
#define RC2_8 0x08
#define RC2_9 0x09

#define RC2_SUBTITLE 0x4B
#define RC2_TELETEXT 0x3C

// RC3-codes
// NOTE: bytes 2 & 0 are a complement of bytes 3 & 1
// where byte 3 is the address and byte 1 is the code
#define RC3_ADDRESS 0x18000000

#define RC3_MUTE 130
#define RC3_ONOFF 98
#define RC3_VOLDOWN1 226
#define RC3_VOLDOWN2 146
#define RC3_VOLUP1 160
#define RC3_VOLUP2 96

#define RC3_AUX 224
#define RC3_LINE 144
#define RC3_OPT 162
#define RC3_COA 192
#define RC3_BLUETOOTH 58

#define RC3_MOVIE 120
#define RC3_MUSIC 122
#define RC3_NEWS 64
#define RC3_PAIR 26

#define RC1B_BITS  12
#define RC2_BITS  20
#define RC3_BITS  32

#define DELAY 100

IRrecv irrecv(RECV_PIN);
IRsend irsend;
bool logging = 0;
unsigned int time1 = 315;
unsigned int time2 = 630;
unsigned long toggleRC1A = 0;
unsigned long toggleRC2 = 0;
bool specialMode = 0;     // 0-button pressed, waiting for another numeric button for special purposes.
bool numericKeyLast = 0;  // Last button pressed was numeric
bool suppressRC1A = 0;    // RC1A Suppressing active, when in certain TV screens (like Smart TV) that require buttons that are also mapped to RC1

int speakerMode = 0;      // Toggle through modes: Optical (default, TV), Bluetooth, AUX, Line, Coax
int teletextMode = 0;     // Toggle through modes: Teletext, Teletext transparent, TV

unsigned long lastKeyTime = 0;
unsigned long lastSpeakerModeTime = 0;
unsigned long lastRC3 = 0;
unsigned long lastValue;
unsigned int repeatCount;
decode_results results;

// RC1: Arcadyan HMB2260 Set-top box
//   RC1A: Unknown protocol, Manchester encoding on 56kHz
void sendRC1A(unsigned long code, bool repeat)
{
  unsigned int buf[45];

  if (repeat)
  {
    if (logging)
    {
      Serial.print(code);
      Serial.print("*");
    }
  }
  else
  {
    if (logging)
    {
      Serial.print(code);
      Serial.print(" ");
    }
    toggleRC1A ^= RC1A_TOGGLE;
    code |= toggleRC1A;
  }
  code |= RC1A_ADDRESS;

  int index = 0;
  bool lastbit = true;
  for (int bitIndex = 21; bitIndex >= 0; bitIndex--)
  {
    bool bt = (code & (1L << bitIndex)) != 0L;
    if (bt == lastbit)
    {
      if (bitIndex < 21) buf[index++] = time1;
      buf[index++] = time1;
    }
    else
    {
      buf[index++] = time2;
    }
    lastbit = bt;
  }
  // Set trailing mark
  buf[index++] = time2;
  if (lastbit)
  {
    buf[index++] = time1;
  }
  irsend.sendRaw(buf, index, 56);
  irrecv.enableIRIn();      // Start the receiver. Needs to be done after an ir-send (???)
}

// RC2: Philips TV (RC6-protocol)
void sendRC2(unsigned long code, bool repeat)
{
  if (repeat)
  {
    if (logging)
    {
      Serial.print(code);
      Serial.print("*");
    }
  }
  else
  {
    if (logging)
    {
      Serial.print(code);
      Serial.print(" ");
    }
    toggleRC2 ^= RC2_TOGGLE;
    code |= toggleRC2;
  }

  irsend.sendRC6(code, RC2_BITS);
  irrecv.enableIRIn();      // Start the receiver. Needs to be done after an ir-send (???)
}

// RC3: Speakerset (NEC-protocol)
void sendRC3(unsigned long code, bool repeat)
{
  if (repeat)
  {
    if (logging)
    {
      Serial.print(code);
      Serial.print("*");
    }
    unsigned int buf[3];
    buf[0] = 9000;  // Mark 9ms
    buf[1] = 2250;  // Space 2.25ms
    buf[2] = 560;   // Burst
    irsend.sendRaw(buf, 3, 38);
  }
  else
  {
    if (logging)
    {
      Serial.print(code);
      Serial.print(" ");
    }
    code = (code << 8) | RC3_ADDRESS;   // Combine with address
    code |= ~(code >> 8 | 0xFF00FF00);  // bytes 3 and 1 are complemented in bytes 2 and 0
    code &= 0x0FFFFFFF;
    //Serial.print(" -------> ");
    //Serial.println(code, HEX);
    irsend.sendNEC(code, RC3_BITS);
  }
  irrecv.enableIRIn();      // Start the receiver. Needs to be done after an ir-send (???)
}

void sendRC3_VolumeUp()
{
  unsigned long code = lastRC3 == RC3_VOLUP1 ? RC3_VOLUP2 : RC3_VOLUP1;
  sendRC3(code, 0);
  lastRC3 = code;
}

void sendRC3_VolumeDown()
{
  unsigned long code = lastRC3 == RC3_VOLDOWN1 ? RC3_VOLDOWN2 : RC3_VOLDOWN1;
  sendRC3(code, 0);
  lastRC3 = code;
}

// RC1: Arcadyan HMB2260 Set-top box
//   RC1B: RC5-protocol 38kHz for TV
// Controls the speakerset
void decodeRC1(unsigned long value, int repeatCount, bool repeat)
{
  switch (value)
  {
    case RC1B_VOLUP:
      sendRC3_VolumeUp(); delay(DELAY);
      sendRC3_VolumeUp();   // Send extra VolumeUp, for faster response
      break;
    case RC1B_VOLDOWN:
      sendRC3_VolumeDown(); delay(DELAY);
      sendRC3_VolumeDown(); // Send extra VolumeDown, for faster response
      break;
  }
  if (!repeat)
  {
    switch (value)
    {
      case RC1B_MUTE:
        sendRC3(RC3_ONOFF, 0); delay(DELAY);   // Turn on/off soundbar
        sendRC3(RC3_OPT, 0); delay(DELAY);     // Select optical
        break;
    }
  }
}

// RC2: Philips TV (RC6-protocol)
// This one controls them all
void decodeRC2(unsigned long value, int repeatCount, bool repeat)
{
  if (value == RC2_GUIDE ||
      value == RC2_SETTINGS ||
      value == RC2_SEARCH ||
      value == RC2_PICFORMAT ||
      value == RC2_LIST ||
      value == RC2_SOURCE ||
      value == RC2_SMARTTV ||
      value == RC2_INFO ||
      value == RC2_OPTIONS)
  {
    suppressRC1A = 1;    // One of these buttons pressed: start suppressing sending codes to RC1A
    digitalWrite(MONITOR_PIN, 1);
  }
  else if (value == RC2_MULTIVIEW ||
           value == RC2_TELETEXT ||
           value == RC2_PROGUP ||
           value == RC2_PROGDOWN ||
           (value == RC2_RETURN && repeatCount > 4))    // Pressing Return longer, stops suppressing as well.
  {
    suppressRC1A = 0;   // One of these buttons pressed: stop suppressing sending codes to RC1A
    digitalWrite(MONITOR_PIN, 0);
  }

  if (value == RC2_ONOFF && repeatCount == 4)     // Press ON/OFF longer to toggle ON/OFF of receiver
  {
    sendRC1A(RC1A_ONOFF, 0);
  }

  if (!repeat)
  {
    switch (value)
    {
      case RC2_ONOFF:
        sendRC1A(RC1A_ONOFF, 0); delay(DELAY);  // Turn on/off Receiver
        sendRC3(RC3_ONOFF, 0);                  // Turn on/of sound bar
        sendRC3(RC3_OPT, 0);                    // Select optical
        break;
      case RC2_MUTE:
        sendRC3(RC3_ONOFF, 0); delay(DELAY);   // Turn on/off sound bar
        sendRC3(RC3_OPT, 0);                   // Select optical
        break;
      case RC2_PROGUP:
        sendRC1A(RC1A_PROGUP, 0);
        suppressRC1A = 0;
        digitalWrite(MONITOR_PIN, 0);
        break;
      case RC2_PROGDOWN:
        sendRC1A(RC1A_PROGDOWN, 0);
        suppressRC1A = 0;
        digitalWrite(MONITOR_PIN, 0);
        break;
      case RC2_TELETEXT:
        teletextMode = (teletextMode + 1) % 3;    // Toggle through modes: Teletext, Teletext transparent, TV
        if (teletextMode == 2)
          sendRC1A(RC1A_TV, 0);  // Leave teletext
        else
          sendRC1A(RC1A_TELETEXT, 0);  // Teletext
        break;
      case RC2_MULTIVIEW:         // We cannot use the TV-Guide button of RC2, because it triggers the TV Guide of then TV itself.
        sendRC1A(RC1A_GUIDE, 0);  // We use the Multi-View button of RC2 instead, which has no function on the TV (as long as we don't use the TV-tuner).
        break;
    }
  }
  switch (value)
  {
    case RC2_VOLUP:
      sendRC3_VolumeUp(); delay(DELAY);
      sendRC3_VolumeUp();   // Send extra VolumeUp, for faster response
      break;
    case RC2_VOLDOWN:
      sendRC3_VolumeDown(); delay(DELAY);
      sendRC3_VolumeDown();   // Send extra sendRC3_VolumeDown, for faster response
      break;
  }

  // Suppress first  repeat, arrow keys tend to act twice unless pressed very shortly
  if (!suppressRC1A && (!repeat || repeatCount > 1))
  {
    switch (value)
    {
      case RC2_UP:
        sendRC1A(RC1A_UP, repeat);
        break;
      case RC2_DOWN:
        sendRC1A(RC1A_DOWN, repeat);
        break;
      case RC2_LEFT:
        sendRC1A(RC1A_LEFT, repeat);
        break;
      case RC2_RIGHT:
        sendRC1A(RC1A_RIGHT, repeat);
        break;
      case RC2_OK:
        sendRC1A(RC1A_OK, repeat);
        break;
      case RC2_RETURN:
        sendRC1A(RC1A_RETURN, repeat);
        break;
      case RC2_STOP:
        sendRC1A(RC1A_STOP, repeat);
        break;
      case RC2_PAUSE:
        sendRC1A(RC1A_PAUSE, repeat);
        break;
      case RC2_REC:
        sendRC1A(RC1A_REC, repeat);
        break;
      case RC2_REWIND:
        sendRC1A(RC1A_REWIND, repeat);
        break;
      case RC2_FF:
        sendRC1A(RC1A_FF, repeat);
        break;
      case RC2_RED:
        sendRC1A(RC1A_RED, repeat);
        break;
      case RC2_GREEN:
        sendRC1A(RC1A_GREEN, repeat);
        break;
      case RC2_YELLOW:
        sendRC1A(RC1A_YELLOW, repeat);
        break;
      case RC2_BLUE:
        sendRC1A(RC1A_BLUE, repeat);
        break;
    }
  }

  if (!repeat)
  {
    // Numeric keys
    if (value < 10)
    {
      unsigned long keyTime = micros();
      bool lastKeyWasNumeric = numericKeyLast && keyTime - lastKeyTime < 2000000;
      numericKeyLast = false;
      if (!lastKeyWasNumeric)
        specialMode = 0;
      if (specialMode)  // Special mode is entered by pressing 0 first
      {
        switch (value)
        {
          case RC2_0:
            sendRC1A(RC1A_MENU, 0);      // Force sending a 0
            break;
          case RC2_1:
            sendRC3(RC3_MOVIE, 0);    // Soundbar in mode 'Movie'
            break;
          case RC2_2:
            sendRC3(RC3_MUSIC, 0);    // Soundbar in mode 'Music'
            break;
          case RC2_3:
            sendRC3(RC3_NEWS, 0);     // Soundbar in mode 'News'
            break;
          case RC2_4:
            if (keyTime - lastSpeakerModeTime > 10000000 && speakerMode != 0)
              speakerMode = 0;
            else
              speakerMode = (speakerMode + 1) % 5;
            switch (speakerMode)
            {
              case 0:
                sendRC3(RC3_OPT, 0);    // Default source
                break;
              case 1:
                sendRC3(RC3_BLUETOOTH, 0);    // Next preferred source
                break;
              case 2:
                sendRC3(RC3_AUX, 0);
                break;
              case 3:
                sendRC3(RC3_LINE, 0);
                break;
              case 4:
                sendRC3(RC3_COA, 0);
                break;
            }
            lastSpeakerModeTime = keyTime;
            break;
          case RC2_5:
            sendRC1A(RC1A_ONOFF, 0);     // Receiver ON/OFF
            break;
          case RC2_6:
            sendRC1A(RC1A_RED, 0);     // Receiver Red
            break;
          case RC2_7:
            sendRC1A(RC1A_GREEN, 0);     // Receiver Green
            break;
          case RC2_8:
            sendRC1A(RC1A_YELLOW, 0);     // Receiver Yellow
            break;
          case RC2_9:
            sendRC1A(RC1A_BLUE, 0);     // Receiver Blue
            break;
            
        }
        specialMode = 0;
      }
      else
      {
        if (value == RC2_0 && !lastKeyWasNumeric)    // 0 pressed, enter special mode
        {
          specialMode = 1;
        }
        else
        {
          // Just send the numeric key (0 is encoded as 0x10 - RC1A_0)
          if (!suppressRC1A) sendRC1A(value == 0 ? RC1A_0 : value, 0);
        }
        numericKeyLast = true;
      }
      lastKeyTime = keyTime;
    }
    else
    {
      numericKeyLast = false;
    }
  }
}

void setup()
{
  pinMode(RECV_PIN, INPUT);
  pinMode(MONITOR_PIN, OUTPUT);
  Serial.begin(19200);
  while (!Serial);
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Started. Send 'L' to enable logging.");
}

void loop() {
  // Check of the sensor:
  // digitalWrite(MONITOR_PIN, digitalRead(RECV_PIN));
  // return;

  if (Serial.available() > 0)
  {
    int serialRead = Serial.read();
    if (serialRead == 'l' || serialRead == 'L')    // Enable logging
    {
      logging = 1 - logging;
      Serial.print("Logging ");
      Serial.println(logging ? "ON" : "OFF");
    }
  }

  if (irrecv.decode(&results))
  {
    if (results.decode_type != UNKNOWN)
    {
      bool repeat = results.value == lastValue;
      repeatCount = repeat ? repeatCount + 1 : 0;
      lastValue = results.value;
      unsigned long value = results.value & 0x7FF;

      // Serial.print(results.decode_type);
      // Serial.print(" - ");

      if (logging)
      {
        Serial.print(value, HEX);
        Serial.print(repeat ? "*" : " ");
        Serial.print(" ---> ");
      }

      switch (results.decode_type)
      {
        case RC5:
          decodeRC1(value, repeatCount, repeat);
          break;

        case RC6:
          decodeRC2(value, repeatCount, repeat);
          break;
      }
      if (logging)
      {
        Serial.println("");
      }
    }
    irrecv.resume(); // Receive the next value
  }
}

