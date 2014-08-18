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

inline void handle_button_press(int button) {
  button_pressed = button;
}

void handle_button_press_0() {
  handle_button_press(0);
}
void handle_button_press_1() {
  handle_button_press(1);
}

typedef struct {
  unsigned int interrupt_number;
  void (*handler)();
} button;

button buttons[2] = {
  {
    .interrupt_number = 0,
    .handler = handle_button_press_0
  },
  {
    .interrupt_number = 1,
    .handler = handle_button_press_1
  }
};

typedef struct {
  unsigned int cs_pin;
  unsigned int dc_pin;
  unsigned int rst_pin;
} screen;

screen screens[2] = {
  {
    .cs_pin = 4,
    .dc_pin = 5,
    .rst_pin = 6
  },
  {
    .cs_pin = 7,
    .dc_pin = 8,
    .rst_pin = 9
  }
};

typedef struct {
  long time_remaining_ms;
  long last_update_ms;
  TFT tft;
  char display_time[12];
  int display_width_chars;
  unsigned int interrupt_number;
  void (* handle_button_press)();
} player;

player players[2] = { 
  {
    .time_remaining_ms = INITIAL_TIME_MS,
    .last_update_ms = 0,
    .tft = TFT(screens[0].cs_pin, screens[0].dc_pin, screens[0].rst_pin),
    .display_time = {},
    .display_width_chars = 0,
    .interrupt_number = buttons[0].interrupt_number,
    .handle_button_press = buttons[0].handler
  },
  {
    .time_remaining_ms = INITIAL_TIME_MS,
    .last_update_ms = 0,
    .tft = TFT(screens[1].cs_pin, screens[1].dc_pin, screens[1].rst_pin),
    .display_time = {},
    .display_width_chars = 0,
    .interrupt_number = buttons[1].interrupt_number,
    .handle_button_press = buttons[1].handler
  }
};

void init_display(player *p) {
  p->tft.begin();
  p->tft.background(0,0,0);
  p->tft.stroke(255,255,255);
  p->tft.fill(0,0,0);
  p->tft.setTextSize(DISPLAY_FONT_SIZE);
}

void update_timer(player *p) {
  p->time_remaining_ms -= (millis() - p->last_update_ms);
  p->last_update_ms = millis();
}

void incrementally_update_text(player *p, const char *text) {
  for(unsigned int i = 0; i < strlen(text); i++) {
    p->tft.text(text, 0, 20);
    if(text[i] != p->display_time[i]) {

      // draw a rect
      p->tft.stroke(0,0,0);
      p->tft.rect(CHAR_WIDTH_PX * i , 20, CHAR_WIDTH_PX, CHAR_HEIGHT_PX);
      //p->tft.background(0,0,0);
      char next_char[2] = {text[i], '\0' };
      p->tft.stroke(255,255,255);
      p->tft.text(next_char, CHAR_WIDTH_PX * i, 20);
    }
  }
  strncpy(p->display_time, text, sizeof(p->display_time));
}

void update_display(player *p) {
  static char timea[12];
  ltoa(p->time_remaining_ms/100, timea, 10);
  incrementally_update_text(p, timea);
}

// does the SPI library allow selecting of which SS to issue the command on?

void setup(void) {
  Serial.begin(115200);
  serprintf("Initializing...");

  for(unsigned int i = 0; i < sizeof(players) / sizeof(players[0]); i++) {
    init_display(&players[i]);
    update_display(&players[i]);
    attachInterrupt(players[i].interrupt_number, players[i].handle_button_press, RISING);
  }

  serprintf("done.\r\n");
  //delay(100);
  //exit(1);
}


int loop_count = 0;
void loop() {
  if (loop_count == 0) {
    serprintf("in loop\r\n");
    loop_count++;
  }

  if (active_player != NONE) {
    // Check for flag
    if (players[active_player].time_remaining_ms <= 0) {
      active_player = NONE;
      button_pressed = NONE;
      return;
    }

    // Update the active player's timer and display
    update_timer(&players[active_player]);
    update_display(&players[active_player]);
  }

  if ((active_player == NONE || button_pressed == active_player) && button_pressed != NONE)   {
    // handle button press
    serprintf("before: active_player=%d, button_pressed=%d\r\n", active_player, button_pressed);
    active_player = (button_pressed + 1) % 2;
    players[active_player].last_update_ms = millis();
    button_pressed = NONE;
    serprintf("after:  active_player=%d, button_pressed=%d\r\n", active_player, button_pressed);
  }     

}

