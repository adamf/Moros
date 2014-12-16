/* 
* TODO:
* reset button (chronos style?) - P1
* time setting - P1
* case - P1
* wiring diagram - P2 S3
* pause - P2
* have graphical representation of flag P3
* OLEDs
* 
*/

#include <SPI.h>
#include <SD.h>
//#include <TFT.h>
#include <FTOLED.h>
#include "moros_oled.h"
#include "printf.h"

#define NONE -1

//#define INITIAL_TIME_MS 300000
#define INITIAL_TIME_MS 25000


class Button {
public:
  int interrupt_number;
  Button(int interrupt_number_, void (*interrupt_handler)()) {
    interrupt_number = interrupt_number_;
    attachInterrupt(interrupt_number, interrupt_handler, RISING);
  };
};

class PollButton {
private:
  bool pressed;
  unsigned long pressed_at;
public:
  unsigned pin_number;
  PollButton(unsigned pin_number_):
    pressed(false),
    pressed_at(0),
    pin_number(pin_number_) {
    pinMode(pin_number, INPUT);
  };

  // returns the number of ms for which the button has been held down
  unsigned long poll() {
    int pin_status = digitalRead(pin_number);
    // serprintf("PollButton::poll: status is %d (%s)\r\n", pin_status, pin_status == HIGH ? "high" : "not high");

    if (pin_status == HIGH) {
      if (!pressed) {
        pressed_at = millis();
        pressed = true;
        serprintf("PollButton::poll: pressed\r\n");
      }

      //serprintf("pressed_at=%lu, millis=%lu\r\n", pressed_at, millis());
      return millis()-pressed_at;
    } else if (pin_status == LOW) {
      if (pressed)
        serprintf("PollButton::poll: released\r\n");
      pressed = false;
      pressed_at = 0;
    } else {
      // ???
      serprintf("PollButton::poll: unrecognized return value from digitalRead: %d\r\n", pin_status);
    }

    return 0;
  }
};

class HumanTime {
public:
   char hours[2];
   char minutes[3];
   char seconds[3];
   char tenths[2];

  HumanTime() {
    set_hours(0);
    set_minutes(0);
    set_seconds(0);
    set_tenths(0);
    serprintf("new time\r\n");
  };

  void set_hours(unsigned int _hours) {
    serprintf("%d\r\n", _hours);
    snprintf(hours, sizeof(hours), "%01d", _hours);
  };
  void set_minutes(unsigned int _minutes) {
    serprintf("%d\r\n", _minutes);
    snprintf(minutes, sizeof(minutes), "%02d", _minutes);
  };
  void set_seconds(unsigned int _seconds) {
    snprintf(seconds, sizeof(seconds), "%02d", _seconds);
    serprintf("%s\r\n", seconds);
  };
  void set_tenths(unsigned int _tenths) {
    snprintf(tenths, sizeof(tenths), "%01d", _tenths);
  }; 
};

class Clock {
private:
  //HumanTime *h_time;
public:
  unsigned long time_remaining_ms;
  unsigned long last_update_ms;
  Clock() { 
//    h_time = new HumanTime();
    init(); 
  };

  void init() {
    time_remaining_ms = INITIAL_TIME_MS;
    last_update_ms = 0;
  }

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

  bool flag() {
    return time_remaining_ms == 0;
  };

  char *human_time_remaining() {
    static char text[8];
    // TODO(adamf) implement hours left correctly so we can play long games.
    unsigned int hours_left = time_remaining_ms / 1000 / 60 / 60;
    if (hours_left < 1) {
        hours_left = 0; 
    }

    // Minutes left is wrong, as it doesn't take into account hours left. 
    unsigned int minutes_left = time_remaining_ms / 1000 / 60;
    unsigned int seconds_left = (time_remaining_ms - (minutes_left * 60000)) / 1000;
    unsigned int tenths_left = (time_remaining_ms - ((minutes_left * 60000) + (seconds_left * 1000))) / 100;
    //serprintf("min %d sec %d tenths %d\r\n", minutes_left, seconds_left, tenths_left);
    snprintf(text, sizeof(text), "%02d:%02d", minutes_left, seconds_left);

    //h_time->set_hours(hours_left);
    //h_time->set_minutes(minutes_left);
    //h_time->set_minutes(minutes_left);
    //h_time->set_tenths(tenths_left);
    //serprintf("hours %s min %s sec %s tenths %s\r\n", h_time->hours, h_time->minutes, h_time->seconds, h_time->tenths);
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
    char main_clock_text[6];
    char secondary_clock_text[3];
  //  HumanTime *h_time = clock->human_time_remaining();
  //  HumanTime *h_time = clock->human_time_remaining();
 //   snprintf(main_clock_text, sizeof(main_clock_text), "%s:%s", h_time->minutes, h_time->seconds);
 //   serprintf("%s - %s - %s", main_clock_text, h_time->minutes, h_time->seconds);
//    snprintf(secondary_clock_text, sizeof(secondary_clock_text), ".%s", h_time->tenths);
//    screen->display_text(main_clock_text);
    screen->display_text(clock->human_time_remaining());
//    screen->display_secondary_text(secondary_clock_text);
  };

  void flag() {
    screen->display_flag();
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
protected:
  const unsigned long reset_button_reset_ms = 3000;
  const unsigned long reset_button_poweroff_ms = 5000;
  enum { INIT, PRE_GAME, IN_PROGRESS, PAUSED } game_state;
public:
  Player *players[2];
  PollButton *reset_button;
  static int active_player;
  volatile static int interrupt_fired;

  Controller() {
    players[0] = new Player(
     new Button(0, handle_interrupt_0),
     new Screen(5,9,10)
    );
    players[1] = new Player(
     new Button(1, handle_interrupt_1),
     new Screen(7,6,8)
    );
    reset_button = new PollButton(A0);
    game_state = INIT;
  }

  void init() {
    for(unsigned int i = 0; i < sizeof(players) / sizeof(players[0]); i++) {
      players[i]->init();
      //players[i]->display_test_pattern();
    }
    reset();
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

  void power_off() {
    //serprintf("Power off\r\n");
  }

  void reset() {
    if (game_state == PRE_GAME) return;

    for(unsigned int i = 0; i < sizeof(players) / sizeof(players[0]); i++) {
      players[i]->clock->init();
      players[i]->screen->reset();
      players[i]->update_display();
    }
    active_player = NONE;
    game_state = PRE_GAME;
    serprintf("Reset\r\n");
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

        if (players[player_who_pressed]->out_of_time()) {
          // the other player is already out of time, so ignore
          active_player = NONE;
        } else {
          // switch to the opposite player from the one whose button was pressed
          active_player = (player_who_pressed+1) % 2;
          game_state = IN_PROGRESS;
          players[active_player]->clock->start();
        }

      } else {
        serprintf("Ignoring button press for inactive player %d\r\n", player_who_pressed);
      }

      interrupt_fired = NONE;
    }

    unsigned long reset_button_held = reset_button->poll();
    //if (reset_button_held > 0)
    //  serprintf("reset button was held for %d\r\n", reset_button_held);
    if (reset_button_held > reset_button_poweroff_ms) {
      power_off();
    } else if (reset_button_held > reset_button_reset_ms) {
      reset();
    }

    if (active_player == NONE)
      // It's nobody's turn, so just keep waiting for a button press
      return;

    // It's someone's turn:

    // Check for flag
    if (players[active_player]->out_of_time()) {
      serprintf("Flag fell for player %d\r\n", active_player);
      players[active_player]->flag();
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
  serprintf("Initializing...\r\n");

  controller.init();

  serprintf("done.\r\n");
}


void loop() {
  controller.tick();

}

