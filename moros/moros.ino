/* 
* TODO:
* case
* flag - P0 BLOCKER S0
* -- have graphical representation of flag
* wiring diagram - P2 S3
* reset button (chronos style?) - P1
* time setting - P1
* OLEDs
* 
*/

#include <SPI.h>
#include <TFT.h>
#include "printf.h"

#define NONE -1

#define DISPLAY_FONT_SIZE 7
#define CHAR_HEIGHT_PX DISPLAY_FONT_SIZE * 10
#define CHAR_WIDTH_PX 42
#define DISPLAY_WIDTH_CHAR 4

//#define INITIAL_TIME_MS 300000
#define INITIAL_TIME_MS 5000


int active_player = NONE;
volatile int button_pressed = NONE;

inline void handle_button_interrupt(int button) {
  button_pressed = button;
}

void handle_button_interrupt_0() {
  handle_button_interrupt(0);
}
void handle_button_interrupt_1() {
  handle_button_interrupt(1);
}

typedef struct {
  unsigned int interrupt_number;
  void (*interrupt_handler)();
} Button;

Button buttons[2] = {
  {
    .interrupt_number = 0,
    .interrupt_handler = handle_button_interrupt_0
  },
  {
    .interrupt_number = 1,
    .interrupt_handler = handle_button_interrupt_1
  }
};

class Screen {
private:
  char prev_text[12];
public:
  TFT *tft;
  Screen(unsigned int cs_pin, unsigned int dc_pin, unsigned int rst_pin) {
    tft = new TFT(cs_pin, dc_pin, rst_pin);
    prev_text[0] = '\0';
  };
  void init() {
    tft->begin();
    tft->background(0,0,0);
    tft->stroke(255,255,255);
    tft->fill(0,0,0);
    tft->setTextSize(DISPLAY_FONT_SIZE);
  };

  void display_text(const char *text) {
    for(unsigned int i = 0; i < strlen(text); i++) {
      if(text[i] != prev_text[i]) {

        // erase this character cell
        tft->stroke(0,0,0);
        tft->rect(CHAR_WIDTH_PX * i , 20, CHAR_WIDTH_PX, CHAR_HEIGHT_PX);

        // print the new character
        char next_char[2] = {text[i], '\0' };
        tft->stroke(255,255,255);
        tft->text(next_char, CHAR_WIDTH_PX * i, 20);
      }
    }
    strncpy(prev_text, text, sizeof(prev_text));
  }
};

Screen *screens[2] = {
  new Screen(4,5,6),
  new Screen(7,8,9)
};

typedef struct {
  unsigned long time_remaining_ms;
  unsigned long last_update_ms;
} Clock;

typedef struct {
  Clock *clock;
  Button *button;
  Screen *screen;
} Player;

Player players[2] = { 
  {
    .clock = new Clock,
    .button = &buttons[0],
    .screen = screens[0]
  },
  {
    .clock = new Clock,
    .button = &buttons[1],
    .screen = screens[1]
  }
};

void init_player(Player *p) {
  p->screen->init();
  Clock *clock = p->clock;
  clock->time_remaining_ms = INITIAL_TIME_MS;
  clock->last_update_ms = 0;
}

void update_timer(Clock *c) {
  unsigned long time_since_last_update_ms = millis() - c->last_update_ms;
  if (c->time_remaining_ms < time_since_last_update_ms) {
    c->time_remaining_ms = 0;
  } else {
    c->time_remaining_ms -= time_since_last_update_ms;
  }
  c->last_update_ms = millis();
}

void update_display(Player *p) {
  static char timea[12];
  ltoa(p->clock->time_remaining_ms/100, timea, 10);
  p->screen->display_text(timea);
}

void handle_button_press(int button) {
  serprintf("button %d pressed\r\n", button);
  serprintf("before: active_player=%d, button_pressed=%d\r\n", active_player, button_pressed);
  active_player = (button_pressed + 1) % 2;
  players[active_player].clock->last_update_ms = millis();
  button_pressed = NONE;
  serprintf("after:  active_player=%d, button_pressed=%d\r\n", active_player, button_pressed);
}

int out_of_time(Player *p) {
  return p->clock->time_remaining_ms <= 0;
}

void setup(void) {
  Serial.begin(115200);
  serprintf("Initializing...");

  for(unsigned int i = 0; i < sizeof(players) / sizeof(players[0]); i++) {
    init_player(&players[i]);
    update_display(&players[i]);
    attachInterrupt(players[i].button->interrupt_number, players[i].button->interrupt_handler, RISING);
  }

  serprintf("done.\r\n");
}


void loop() {
  if (active_player != NONE) {
    if (out_of_time(&players[active_player])) {
      serprintf("Flag fell for player %d\r\n", active_player);
      active_player = NONE;
      button_pressed = NONE;
      return;
    }

    update_timer(players[active_player].clock);
    update_display(&players[active_player]);
  }

  if ((active_player == NONE || button_pressed == active_player) && button_pressed != NONE)   {
    handle_button_press(button_pressed);
  }     

}

