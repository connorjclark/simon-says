// first line can't be an include for some reason
#include "elapsedMillis/elapsedMillis.h"
#include "SparkButton/SparkButton.h"

/*

When game ends, the lights represent the binary value of the level you got to.
Press two buttons to restart game.

*/

SparkButton b;

float timeLeft;
float timeToEnter = 3000;
float timeToShowStep = 750;
int* buttonLights;

void setup() {
  //Serial.begin(9600);
  //while(!Serial.available()) SPARK_WLAN_Loop();
  
  b.begin();
  
  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);
  pinMode(D4, INPUT_PULLUP);
  
  buttonLights = (int *) calloc(12, sizeof(int));
  buttonLights[0] = 11;
  buttonLights[1] = -1;
  buttonLights[2] = 1;
  buttonLights[3] = 2;
  buttonLights[4] = 3;
  buttonLights[5] = 4;
  buttonLights[6] = 5;
  buttonLights[7] = 6;
  buttonLights[8] = 7;
  buttonLights[9] = 8;
  buttonLights[10] = 9;
  buttonLights[11] = 10;
}

int WAITING_TO_BEGIN = 0;
int SHOWING_PATTERN = 1;
int WAITING_FOR_INPUT = 2;
int GAME_OVER = 3;
int SLEEPING = 4;

int state = WAITING_TO_BEGIN;
int stateToWakeTo;
int pattern[255];
int patternLength = 0;
int showingPattern = 0;

struct ButtonState {
    bool down;
    bool released;
};

struct ButtonState buttonStates[4];

void loop() {
  handleButtons();

  if (state == WAITING_TO_BEGIN) {
      for (int i = 1; i <= 4; i++) lightUpSide(i, (int)timeLeft % 12);
      timeLeft += elapsedMicros()/10.0;
      if (getFirstButtonReleased()) {
        timeLeft = timeToShowStep;
        patternLength = 0;
        for (int i = 0; i < 3; i++) {
            addStepToPattern();
        }
        state = SHOWING_PATTERN;
        showingPattern = 0;
      }
  } else if (state == SLEEPING) {
    b.allLedsOff();
    timeLeft -= elapsedMicros() * 10;
    if (timeLeft <= 0) {
        timeLeft = getTimeToShowStep();
        state = stateToWakeTo;
    }
  } else if (state == SHOWING_PATTERN) {
      b.allLedsOff();
      lightUpSide(pattern[showingPattern], 0);
      
      timeLeft -= elapsedMicros() * 10;
      if (timeLeft <= 0) {
          timeLeft = getTimeToSleep();
          showingPattern++;
          state = SLEEPING;
          stateToWakeTo = SHOWING_PATTERN;
          if (showingPattern >= patternLength) {
              state = WAITING_FOR_INPUT;
              showingPattern = 0;
              timeLeft = getTimeToEnter();
          }
      }
  } else if (state == WAITING_FOR_INPUT) {
      b.allLedsOff();
      
      int buttonReleased = getFirstButtonReleased();
      if (buttonReleased) {
          if (buttonReleased == pattern[showingPattern]) {
              showingPattern++;
              if (showingPattern >= patternLength) {
                  addStepToPattern();
                  state = SHOWING_PATTERN;
                  showingPattern = 0;
                  timeLeft = getTimeToShowStep();
              } else {
                  timeLeft = getTimeToEnter();
              }
          } else {
              state = GAME_OVER;
          }
      }
      
      // show timer on lights
      int numLights = timeLeft / getTimeToEnter() * 11;
      for (int i = 0; i < numLights; i++) {
          int a = 50*(i / 11.0 + 0.05);
          b.ledOn(i + 1, a, a, a);
      }
      
      timeLeft -= elapsedMicros() * 10;
      if (timeLeft < 0) {
          state = GAME_OVER;
      }
  } else if (state == GAME_OVER) {
      displayInt(getRound());
      if (numButtonsDown() > 1) {
        state = SLEEPING;
        stateToWakeTo = WAITING_TO_BEGIN;
        timeLeft = 500;
      }
  }
}

void displayInt(int num) {
    b.allLedsOff();
    for (int i = 1, j = 1; j <= 11; i <<= 1, j++) {
        if (num & i) {
            light(j, 255, 0, 0);
        }
    }
}

int numButtonsDown() {
    int num = 0;
    for (int i = 0; i < 4; i++) num += buttonStates[i].down;
    return num;
}

int getTimeToEnter() {
    int round = getRound();
    return timeToEnter - round * 200;
}

int getTimeToShowStep() {
    int round = getRound();
    return max(250, timeToShowStep - round * 100);
}

int getTimeToSleep() {
    return getTimeToShowStep() / 2;
}

int getRound() {
    return patternLength - 2;
}

void handleButtons() {
    for (int i = 0; i < 4; i++) {
        ButtonState* bs = &buttonStates[i];
        bool wasDown = bs->down;
        bs->down = b.buttonOn(i + 1);
        bs->released = wasDown && !bs->down;
    }
}

int getFirstButtonReleased() {
    for (int i = 0; i < 4; i++) {
        if (buttonStates[i].released) {
            return i + 1;
        }
    }
    return 0;
}

float brightness = 0.125;

void lightUpSide(int button, int offset) {
  int idx = button - 1;
  for (int i = idx * 3; i < (idx + 1) * 3; i++) {
    int led = buttonLights[i];
    if (led == -1) continue;
    led = (led + offset) % 12;
    if (led == 0) led = 1;
    if (button == 1) {
      light(led, 255, 0, 0);
    } else if (button == 2) {
      light(led, 0, 255, 0);
    } else if (button == 3) {
      light(led, 0, 0, 255);
    } else if (button == 4) {
      light(led, 255, 255, 255);
    }
  }
}

void light(int id, int r, int g, int bb) {
    b.ledOn((int) id, (int)(r * brightness), (int)(g * brightness), (int)(bb * brightness));
}

void addStepToPattern() {
    pattern[patternLength++] = (rand() % 4) + 1;
}

void clearPattern() {
    patternLength = 0;
}
