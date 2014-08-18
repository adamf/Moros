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

//#define INITIAL_TIME_MS 300000
#define INITIAL_TIME_MS 5000


class Button {
public:
  static volatile int button_pressed;

  Button(int interrupt_number, void (*interrupt_handler)()) {
    attachInterrupt(interrupt_number, interrupt_handler, RISING);
  };

  static inline void handle_button_interrupt(int button);
};
volatile int Button::button_pressed = NONE;

void handle_button_interrupt_0() {
  Button::handle_button_interrupt(0);
}
void handle_button_interrupt_1() {
  Button::handle_button_interrupt(1);
}

class Screen {
private:
  char prev_text[12];

#define DISPLAY_FONT_SIZE 7
#define CHAR_HEIGHT_PX DISPLAY_FONT_SIZE * 10
#define CHAR_WIDTH_PX 42
#define DISPLAY_WIDTH_CHAR 4

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

class Clock {
public:
  unsigned long time_remaining_ms;
  unsigned long last_update_ms;
  Clock() {
    time_remaining_ms = INITIAL_TIME_MS;
    last_update_ms = 0;
  };

  void start() {
    last_update_ms = millis();
  };

  void update() {
    unsigned long time_since_last_update_ms = millis() - last_update_ms;
    if (time_remaining_ms < time_since_last_update_ms) {
      time_remaining_ms = 0;
    } else {
      time_remaining_ms -= time_since_last_update_ms;
    }
    last_update_ms = millis();
  };
};

class Player {
public:
  Clock *clock;
  Button *button;
  Screen *screen;

  Player(Button *button_, Screen *screen_) {
    clock = new Clock();
    button = button_;
    screen = screen_;
  };

  void init() {
    screen->init();
  };

  void update_display() {
    static char timea[12];
    ltoa(clock->time_remaining_ms/100, timea, 10);
    screen->display_text(timea);
  };

  void tick() {
    clock->update();
    update_display();
  };


  int out_of_time() {
    return clock->time_remaining_ms <= 0;
  }
};

class Controller {
public:
#define PLAYERS 2
  Player *players[PLAYERS];
  static int active_player;

  Controller() {
    players[0] = new Player(
     new Button(0, handle_button_interrupt_0),
     new Screen(4,5,6)
    );
    players[1] = new Player(
     new Button(1, handle_button_interrupt_1),
      new Screen(7,8,9)
    );

  }

  void init() {
    for(unsigned int i = 0; i < sizeof(players) / sizeof(players[0]); i++) {
      players[i]->init();
      players[i]->clock->start();
      players[i]->update_display();
    }
  };

  void handle_button_interrupt(int button) {
    Button::button_pressed = button;
  };

  void handle_button_press(int button) {
    serprintf("button %d pressed\r\n", button);
    serprintf("before: active_player=%d, button_pressed=%d\r\n", active_player, Button::button_pressed);
    active_player = (Button::button_pressed + 1) % 2;
    players[active_player]->clock->last_update_ms = millis();
    Button::button_pressed = NONE;
    serprintf("after:  active_player=%d, button_pressed=%d\r\n", active_player, Button::button_pressed);
  }

  void tick() {
    if (active_player != NONE) {
      if (players[active_player]->out_of_time()) {
        serprintf("Flag fell for player %d\r\n", active_player);
        active_player = NONE;
        Button::button_pressed = NONE;
        return;
      }

      players[active_player]->tick();
    }

    if ((active_player == NONE || Button::button_pressed == active_player) && Button::button_pressed != NONE)   {
      handle_button_press(Button::button_pressed);
    }     
  }
};
int Controller::active_player = NONE;

Controller controller;

void Button::handle_button_interrupt(int button) {
  controller.handle_button_interrupt(button);
};

void setup(void) {
  Serial.begin(115200);
  serprintf("Initializing...");

  controller.init();

  serprintf("done.\r\n");
}


void loop() {
  controller.tick();

}

