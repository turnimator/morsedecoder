struct MorseCode {
  const char c;   // The ASCII character
  const char *s;  // The code as a string of . (dot) and - (dash)
  const char dot; // what ASCII character do you get when adding a dot
  const char dash; // what ASCII character do you get when adding a dash
};

int code = 0;

static const struct MorseCode codeTable[48] = {
  { 'A', ".-", 'R', 'W'},
  { 'B', "-...", 0, 0},
  { 'C', "-.-.", 0, 0},
  {'D', "-..", 'B', 'X'},
  {'E', ".", 'I', 'A'},
  {'F', "..-.", 0, 0},
  {'G', "--.", 'Z', 'Q'},
  {'H', "....", 0, 0},
  {'I', "..", 'S', 'U'},
  {'J', ".---", 0, 0},
  {'K', "-.-", 'C', 'Y'},
  {'L', ".-..", 0, 0},
  {'M', "--", 'G', 'O'},
  {'N', "-.", 'D', 'K'},
  {'O', "---", 0, 0},
  {'P', ".--.", 0, 0},
  {'Q', "--.-", 0, 0},
  {'R', ".-.", 'L', 0},
  {'S' , "...", 'H', 'V'},
  {'T', "-", 'N', 'M'},
  {'U', "..-", 'F', 0},
  {'V', "...-", 0, 0},
  {'W', ".--", 'J', 'P'},
  {'X', "-..-", 0, 0},
  {'Y', "-.--", 0, 0},
  {'Z', "--..", 0, 0}
};


static inline void dot()
{
  int nextCode;
  if (code == 0) {
    nextCode = 'E';
  } else {
    nextCode = codeTable[code - 'A'].dot;
  }
  if (nextCode == 0) {
    space();
  }
  code = nextCode;
}

static inline void dash()
{
  int nextCode;
  if (code == 0) {
    nextCode = 'T';
  } else {
    nextCode = codeTable[code - 'A'].dash;
  }
  if (nextCode == 0) {
    space();
  }
  code = nextCode;
}

static inline void space()
{
  Serial.printf("(%c)\n", code);
  code = 0;
}




void setup() {
  Serial.begin(115200);
  pinMode(4, INPUT);
  for (int i = 0 ; i <= ('Z' - 'A'); i++) {
    Serial.printf("%c, %5s, %c, %c\n",
                  codeTable[i].c,
                  codeTable[i].s,
                  codeTable[i].dot ? codeTable[i].dot : ' ',
                  codeTable[i].dash ? codeTable[i].dash : ' ');
  }
}

int longest_beep = 0;
int shortest_beep = 100000;
int longest_silence = 0;
int shortest_silence = 0;

int beep_len = 0;
int silence_len = 0;

enum State { beeping, silent, idle} state = silent;

void emitDash()
{
  Serial.print("-");
  dash();
}

void emitDot() {
  Serial.print(".");
  dot();
}

void emitSpace()
{
  Serial.print(" ");
  space();
}

/**
   A beep is either a dot or a dash
   A longer than average beep is a dash
   A shorter than average beep is a dot
*/
void decodeBeep()
{
  if (beep_len > longest_beep) {
    longest_beep = beep_len;
  }
  if (beep_len < shortest_beep) {
    shortest_beep = beep_len;
  }
  if (beep_len > (longest_beep + shortest_beep) / 2) {
    emitDash();
    longest_beep = (beep_len + longest_beep) / 2;
  }
  else {
    emitDot();
    shortest_beep = (beep_len + shortest_beep) / 2 ;
  }
  beep_len = 0; // Start new beep
}

/**
   A silence is either a space between beeps or a space between complete codes
   A long silence is a space between codes
   A short silence is a space between beeps
*/
void decodeSilence() {

  if (silence_len > longest_silence) {
    longest_silence = silence_len;
  }
  if (silence_len < shortest_silence) {
    shortest_silence = silence_len;
  }

  if (silence_len > longest_beep) {
    emitSpace();
    longest_silence = (longest_silence + silence_len) / 2;
  } else {
    shortest_silence = (shortest_silence + silence_len) / 2;
  }
  silence_len = 0; // reset silence
}

void loop() {

  byte b = digitalRead(4);
  /*
     If we're beeping, keep on beeping
  */
  switch (state) {
    case beeping:
      if (b) { // still beeping
        beep_len++;
      } else { // stopped beeping
        decodeBeep();
        state = silent;
      }
      break;
    case silent:
      if (b) {// started beeping
        decodeSilence();
        state = beeping;
      } else { // still silent
        if (silence_len > (longest_beep * 2)) { // Timeout
          decodeSilence();
          state = idle;
        }
        silence_len++;

      }
      break;
    case idle:
      if (b) {
        state = beeping;
      }

  }
}
