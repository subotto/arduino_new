#define RED_GOAL          22
#define RED_SUPERGOAL     23
#define RED_PLUS_ONE      24
#define RED_MINUS_ONE     25

#define BLUE_GOAL         26
#define BLUE_SUPERGOAL    27
#define BLUE_PLUS_ONE     28
#define BLUE_MINUS_ONE    29

#define RED_TEAM          0
#define BLUE_TEAM         1

#define VOID              0
#define GOAL              1
#define SUPERGOAL         2
#define PLUS_ONE          3
#define MINUS_ONE         4

#define PORTNO sizeof(ports)/sizeof(short)

static const short ports[] = {
    RED_GOAL, RED_SUPERGOAL, RED_PLUS_ONE, RED_MINUS_ONE,
    BLUE_GOAL, BLUE_SUPERGOAL, BLUE_PLUS_ONE, BLUE_MINUS_ONE
};

static bool is_disabled[PORTNO] = {};

int red_score, blue_score;

unsigned long last_millis[PORTNO] = {};
unsigned long cur_millis[PORTNO] = {};
char last_read[PORTNO] = {};
static const int interval = 500;

typedef void (*event_callback)(void);


void parse_command(byte command) {  
  switch ( command ) {
    case 0:
      red_score--;
      write_display(RED_TEAM, red_score);
      break;
    case 1:
      red_score++;
      write_display(RED_TEAM, red_score);
      break;
    case 2:
      blue_score--;
      write_display(BLUE_TEAM, blue_score);
      break;
    case 3:
      blue_score++;
      write_display(BLUE_TEAM, blue_score);
      break;
    case 4:
    case 5:
      enqueue_update(red_score, RED_TEAM, VOID);
      break;
    case 6:
    case 7:
      enqueue_update(blue_score, BLUE_TEAM, VOID);
      break;
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
      is_disabled[(command & 14) >> 1] = command & 1;
      break;
    default:
      Serial.println("Invalid command");
  }
}


void red_goal() {
  red_score++;
  write_display(RED_TEAM, red_score);
  enqueue_update(red_score, RED_TEAM, GOAL);
}

void red_supergoal() {
  red_score++;
  write_display(RED_TEAM, red_score);
  enqueue_update(red_score, RED_TEAM, SUPERGOAL);
}

void red_plus_button() {
  red_score++;
  write_display(RED_TEAM, red_score);
  enqueue_update(red_score, RED_TEAM, PLUS_ONE);
}

void red_minus_button() {
  red_score--;
  write_display(RED_TEAM, red_score);
  enqueue_update(red_score, RED_TEAM, MINUS_ONE);
}

void blue_goal() {
  blue_score++;
  write_display(BLUE_TEAM, blue_score);
  enqueue_update(blue_score, BLUE_TEAM, GOAL);
}

void blue_supergoal() {
  blue_score++;
  write_display(BLUE_TEAM, blue_score);
  enqueue_update(blue_score, BLUE_TEAM, SUPERGOAL);
}

void blue_plus_button() {
  blue_score++;
  write_display(BLUE_TEAM, blue_score);
  enqueue_update(blue_score, BLUE_TEAM, PLUS_ONE);
}

void blue_minus_button() {
  blue_score--;
  write_display(BLUE_TEAM, blue_score);
  enqueue_update(blue_score, BLUE_TEAM, MINUS_ONE);
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
  init_ethernet();
  Serial.begin(9600);
  init_lcd();
}

void loop() {
  unsigned long m = millis();
  for (int i=0; i<PORTNO; i++)
    if (digitalRead(ports[i]) && !is_disabled[i]) {
      cur_millis[i] = m;
      last_read[i] = 1;
    } else {
      last_read[i] = 0;
    }
  for (int i=0; i<PORTNO; i++) {
    if (cur_millis[i] > last_millis[i] + interval && !last_read[i]) {
      callbacks[i]();
      last_millis[i] = cur_millis[i];
    }
  }
  int command = check_ethernet();
  if (command != -1) {
    Serial.println(command);
    parse_command(command);
  }
  send_event();
}
