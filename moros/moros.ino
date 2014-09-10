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
  int interrupt_number;
  Button(int interrupt_number_, void (*interrupt_handler)()) {
    interrupt_number = interrupt_number_;
    attachInterrupt(interrupt_number, interrupt_handler, RISING);
  };
};

struct Font {
  unsigned int font_size;
  unsigned int char_height_px;
  unsigned int char_width_px;
};

Font fonts[4] = {
  {
    .font_size = 7,
    .char_height_px = 48,
    .char_width_px = 36
  },
  {
    .font_size = 6,
    .char_height_px = 42,
    .char_width_px = 30
  },
  {
    .font_size = 5,
    .char_height_px = 36,
    .char_width_px = 24
  },
  {
    .font_size = 4,
    .char_height_px = 30,
    .char_width_px = 22
  }
};
#define FONT 3

class Screen {
private:
  char prev_text[12];

public:
  TFT *tft;
  Screen(unsigned int cs_pin, unsigned int dc_pin, unsigned int rst_pin) {
    tft = new TFT(cs_pin, dc_pin, rst_pin);
    memset(prev_text, 0, sizeof(prev_text));
  };
  void init() {
    tft->begin();
    tft->background(0,0,0);
    tft->stroke(255,255,255);
    tft->fill(0,0,0);
    tft->setTextSize(fonts[FONT].font_size);
  };

  void display_text(const char *text) {
    for(unsigned int i = 0; i < strlen(text); i++) {
      // serprintf("Considering displaying character: %c\r\n", text[i]);
      if(prev_text[0] == '\0' || text[i] != prev_text[i]) {
        // serprintf("Previous character was: %c\r\n", prev_text[i]);

        // erase this character cell
        // tft->stroke(255,0,0); // draw bounding box
        tft->stroke(0,0,0);
        tft->rect(fonts[FONT].char_width_px * i, 20, fonts[FONT].char_width_px, fonts[FONT].char_height_px);

        // print the new character
        char next_char[2] = {text[i], '\0' };
        tft->stroke(255,255,255);
        tft->text(next_char, i * fonts[FONT].char_width_px, 20);
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

  char *human_time_remaining() {
    static char text[8];
    unsigned int minutes_left = time_remaining_ms / 1000 / 60;
    unsigned int seconds_left = (time_remaining_ms - (minutes_left * 60000)) / 1000;
    unsigned int tenths_left = (time_remaining_ms - ((minutes_left * 60000) + (seconds_left * 1000))) / 100;
    //serprintf("min %d sec %d tenths %d\r\n", minutes_left, seconds_left, tenths_left);
    snprintf(text, sizeof(text), "%02d:%02d.%d", minutes_left, seconds_left, tenths_left);
    //serprintf("%s\r\n", text);
    return text;
  }

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
    //static char timea[12];
    //ltoa(clock->time_remaining_ms/100, timea, 10);
    screen->display_text(clock->human_time_remaining());
  };

  void display_test_pattern() {
    screen->display_text("45:01.5");
  };

  void tick() {
    clock->update();
    update_display();
  };


  int out_of_time() {
    return clock->time_remaining_ms <= 0;
  }
};

void handle_interrupt_0();
void handle_interrupt_1();

class Controller {
public:
  Player *players[2];
  static int active_player;
  volatile static int interrupt_fired;

  Controller() {
    players[0] = new Player(
     new Button(0, handle_interrupt_0),
     new Screen(4,5,6)
    );
    players[1] = new Player(
     new Button(1, handle_interrupt_1),
     new Screen(7,8,9)
    );

  }

  void init() {
    for(unsigned int i = 0; i < sizeof(players) / sizeof(players[0]); i++) {
      players[i]->init();
      players[i]->clock->start();
      players[i]->update_display();
      //players[i]->display_test_pattern();
    }
  };

  // called from the actual interrupt handler. be quick.
  static void handle_interrupt(int interrupt_number) {
    interrupt_fired = interrupt_number;
  };

  int interrupt_to_player(int interrupt_number) {
    for(unsigned int i = 0; i < sizeof(players) / sizeof(players[0]); i++) {
      if (players[i]->button->interrupt_number == interrupt_number)
        return i;
    }
    return NONE;
  }

  void tick() {
    if (interrupt_fired != NONE) {
      // Someone pressed a button since the last time we checked.
      int player_who_pressed = interrupt_to_player(interrupt_fired);

      if (player_who_pressed == NONE) {
        // Unrecognized button interrupt
        serprintf("received interrupt %d, but no player has a button on that interrupt!\r\n", interrupt_fired);

      } else if (active_player == NONE || player_who_pressed == active_player) {
        // either it was nobody's turn, or the active player pressed their button
        // switch to the opposite player from the one whose button was pressed
        active_player = (player_who_pressed+1) % 2;
        players[active_player]->clock->start();

      } else {
        serprintf("Ignoring button press for inactive player %d\r\n", player_who_pressed);
      }

      interrupt_fired = NONE;
    }

    if (active_player == NONE)
      // It's nobody's turn, so just keep waiting for a button press
      return;

    // It's someone's turn:

    // Check for flag
    if (players[active_player]->out_of_time()) {
      serprintf("Flag fell for player %d\r\n", active_player);
      active_player = NONE;
      return;
    }

    // Update the clock
    players[active_player]->tick();
  }
};
int Controller::active_player = NONE;
volatile int Controller::interrupt_fired = NONE;

Controller controller;

// We need a separate function for each interrupt because we can't pass any
// context to the interrupt handler
void handle_interrupt_0() {
  Controller::handle_interrupt(0);
}
void handle_interrupt_1() {
  Controller::handle_interrupt(1);
}

void setup(void) {
  Serial.begin(115200);
  serprintf("Initializing...");

  controller.init();

  serprintf("done.\r\n");
}


void loop() {
  controller.tick();

}

