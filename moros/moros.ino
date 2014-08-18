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

typedef struct {
  unsigned int cs_pin;
  unsigned int dc_pin;
  unsigned int rst_pin;
  TFT *tft;
} Screen;

Screen screens[2] = {
  {
    .cs_pin = 4,
    .dc_pin = 5,
    .rst_pin = 6,
  },
  {
    .cs_pin = 7,
    .dc_pin = 8,
    .rst_pin = 9,
  }
};

typedef struct {
  long time_remaining_ms;
  long last_update_ms;
  char display_time[12];
  int display_width_chars;
  Button *button;
  Screen *screen;
} player;

player players[2] = { 
  {
    .time_remaining_ms = INITIAL_TIME_MS,
    .last_update_ms = 0,
    .display_time = {},
    .display_width_chars = 0,
    .button = &buttons[0],
    .screen = &screens[0]
  },
  {
    .time_remaining_ms = INITIAL_TIME_MS,
    .last_update_ms = 0,
    .display_time = {},
    .display_width_chars = 0,
    .button = &buttons[1],
    .screen = &screens[1]
  }
};

void init_display(player *p) {
  Screen *screen = p->screen;
  screen->tft = new TFT(screen->cs_pin, screen->dc_pin, screen->rst_pin);
  screen->tft->begin();
  screen->tft->background(0,0,0);
  screen->tft->stroke(255,255,255);
  screen->tft->fill(0,0,0);
  screen->tft->setTextSize(DISPLAY_FONT_SIZE);
}

void update_timer(player *p) {
  p->time_remaining_ms -= (millis() - p->last_update_ms);
  p->last_update_ms = millis();
}

void update_changed_chars(player *p, const char *text) {
  Screen *screen = p->screen;
  for(unsigned int i = 0; i < strlen(text); i++) {
    screen->tft->text(text, 0, 20);
    if(text[i] != p->display_time[i]) {

      // draw a rect
      screen->tft->stroke(0,0,0);
      screen->tft->rect(CHAR_WIDTH_PX * i , 20, CHAR_WIDTH_PX, CHAR_HEIGHT_PX);
      //p->tft->background(0,0,0);
      char next_char[2] = {text[i], '\0' };
      screen->tft->stroke(255,255,255);
      screen->tft->text(next_char, CHAR_WIDTH_PX * i, 20);
    }
  }
  strncpy(p->display_time, text, sizeof(p->display_time));
}

void update_display(player *p) {
  static char timea[12];
  ltoa(p->time_remaining_ms/100, timea, 10);
  update_changed_chars(p, timea);
}

void handle_button_press(int button) {
  serprintf("button %d pressed\r\n", button);
  serprintf("before: active_player=%d, button_pressed=%d\r\n", active_player, button_pressed);
  active_player = (button_pressed + 1) % 2;
  players[active_player].last_update_ms = millis();
  button_pressed = NONE;
  serprintf("after:  active_player=%d, button_pressed=%d\r\n", active_player, button_pressed);
}

int out_of_time(player *p) {
  return p->time_remaining_ms <= 0;
}

void setup(void) {
  Serial.begin(115200);
  serprintf("Initializing...");

  for(unsigned int i = 0; i < sizeof(players) / sizeof(players[0]); i++) {
    init_display(&players[i]);
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

    update_timer(&players[active_player]);
    update_display(&players[active_player]);
  }

  if ((active_player == NONE || button_pressed == active_player) && button_pressed != NONE)   {
    handle_button_press(button_pressed);
  }     

}

