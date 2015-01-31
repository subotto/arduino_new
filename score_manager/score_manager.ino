#define RED_GOAL          22
#define RED_SUPERGOAL     23
#define RED_PLUS_ONE      24
#define RED_MINUS_ONE     25

#define BLUE_GOAL         26
#define BLUE_SUPERGOAL    27
#define BLUE_PLUS_ONE     28
#define BLUE_MINUS_ONE    29

#define RED_SCORE         0
#define BLUE_SCORE        1

#define PORTNO sizeof(ports)/sizeof(short)

static const short ports[] = {
    RED_GOAL, RED_SUPERGOAL, RED_PLUS_ONE, RED_MINUS_ONE,
    BLUE_GOAL, BLUE_SUPERGOAL, BLUE_PLUS_ONE, BLUE_MINUS_ONE
};

int red_score, blue_score;

unsigned long last_millis[PORTNO] = {};
unsigned long cur_millis[PORTNO] = {};
char last_read[PORTNO] = {};
static const int interval = 500;

typedef void (*event_callback)(void);

void red_goal() {
  red_score++;
  write_display(RED_SCORE, red_score);
}

void red_supergoal() {
  red_score++;
  write_display(RED_SCORE, red_score);
}

void red_plus_button() {
  red_score++;
  write_display(RED_SCORE, red_score);
}

void red_minus_button() {
  red_score--;
  write_display(RED_SCORE, red_score);
}

void blue_goal() {
  blue_score++;
  write_display(BLUE_SCORE, blue_score);
}

void blue_supergoal() {
  blue_score++;
  write_display(BLUE_SCORE, blue_score);
}

void blue_plus_button() {
  blue_score++;
  write_display(BLUE_SCORE, blue_score);
}

void blue_minus_button() {
  blue_score--;
  write_display(BLUE_SCORE, blue_score);
}

event_callback callbacks[PORTNO] = {
  red_goal, red_supergoal, red_plus_button, red_minus_button,
  blue_goal, blue_supergoal, blue_plus_button, blue_minus_button
};

int val = 0;

void setup() {
  for (int i=0; i<PORTNO; i++) {
    pinMode(ports[i], INPUT);
    last_millis[i] = millis();
  }
  init_lcd();
}

void loop() {
  int m = millis();
  for (int i=0; i<PORTNO; i++)
    if (digitalRead(ports[i])) {
      cur_millis[i] = m;
      last_read[i] = 1;
    } else {
      last_read[i] = 0;
    }
  for (int i=0; i<PORTNO; i++)
    if (cur_millis[i] > last_millis[i] + interval && !last_read[i]) {
      callbacks[i]();
      last_millis[i] = cur_millis[i];
    }
}
