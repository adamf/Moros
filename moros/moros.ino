/* 
* TODO:
* Fix screen staying blank if you switch clock being set in the middle of a blink - P2
* Remove duplicate state in PollButton
* Remove duplicate interrupt checking code from handle_player_buttons*
* case - P1
* display tenths of a second - P2
* 
*/

#include <SPI.h>
#include <FTOLED.h>
#include "moros_oled.h"
#include "printf.h"

#define NUM_PLAYERS 2
#define NONE -1

//#define INITIAL_TIME_MS 300000
#define INITIAL_TIME_MS 25000


class ButtonState {
public:
  bool pressed = false;
  unsigned long press_duration_ms = 0;
  unsigned long pressed_at = 0;
  bool just_released = false;
};

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
  ButtonState *state;
public:
  unsigned pin_number;
  PollButton(unsigned pin_number_):
    pressed(false),
    pressed_at(0),
    pin_number(pin_number_) {
    state = new ButtonState();
    state->pressed = false;
    state->press_duration_ms = 0;
    state->pressed_at = 0;
    state->just_released = false;
    pinMode(pin_number, INPUT);
    digitalWrite(pin_number, HIGH); // connect the internal pull-up resistor
  };

  // returns the number of ms for which the button has been held down, and if the button is still pressed.
  ButtonState* poll() {
    int pin_status = digitalRead(pin_number);
    state->just_released = false;
    // serprintf("PollButton::poll: status is %d (%s)\r\n", pin_status, pin_status == HIGH ? "high" : "not high");

    if (pin_status == LOW) {
      if (!pressed) {
        pressed_at = millis();
        pressed = true;
        state->pressed = true;
        state->pressed_at = pressed_at;
        serprintf("PollButton::poll: pressed\r\n");
      }

      //serprintf("pressed_at=%lu, millis=%lu\r\n", pressed_at, millis());
//      serprintf("pressed_at=%lu, pressed=%d\r\n", state.press_duration_ms, state.pressed);
      //return millis()-pressed_at;
      state->press_duration_ms = millis() - pressed_at;
      return state;
    } else if (pin_status == HIGH) {
      if (pressed)
        serprintf("PollButton::poll: released\r\n");
      pressed = false;
      pressed_at = 0;
      state->pressed = false;
      state->just_released = true;
      state->press_duration_ms = 0;
      return state;
    } else {
      // ???
      serprintf("PollButton::poll: unrecognized return value from digitalRead: %d\r\n", pin_status);
    }

    //return 0;
    // XXX Should we reset state before returning it here?
    return state;
  }
};

class Clock {
public:
  unsigned long time_remaining_ms;
  unsigned long last_update_ms;
  unsigned long starting_time_ms;

  Clock() { 
    starting_time_ms = INITIAL_TIME_MS;
    time_remaining_ms = starting_time_ms;
    last_update_ms = 0;
  };

  void init() {
    //serprintf("Clock init...\r\n");
    time_remaining_ms = starting_time_ms;
    last_update_ms = 0;
    //serprintf("Clock init complete.\r\n");
  }

  void start() {
    last_update_ms = millis();
  };

  void update() {
    if (time_remaining_ms == 0) return;
    unsigned long time_since_last_update_ms = millis() - last_update_ms;
    if (time_remaining_ms < time_since_last_update_ms) {
      time_remaining_ms = 0;
    } else {
      time_remaining_ms -= time_since_last_update_ms;
    }
    last_update_ms = millis();
  };

  void add_minute() {
    if (starting_time_ms >= 99UL * 60 * 1000) {
      starting_time_ms = 99UL* 60 * 1000;
    } else {
      starting_time_ms = starting_time_ms + 60000;
    }
    time_remaining_ms = starting_time_ms;
  };

  void subtract_minute() {
    if (starting_time_ms <= 60000) {
      starting_time_ms = 60000;
    } else {
      starting_time_ms = starting_time_ms - 60000;
    }
    time_remaining_ms = starting_time_ms;
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
    serprintf("Player init...\r\n");
    screen->init();
    serprintf("Player init complete.\r\n");
  };

  void update_display() {
    screen->display_text(clock->human_time_remaining());
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

  void blank_text() {
    screen->blank_text();
  };


  int out_of_time() {
    return clock->time_remaining_ms <= 0;
  }
};

void handle_interrupt_0();
void handle_interrupt_1();

class Controller {
protected:
  const unsigned long mode_button_reset_ms = 1000;
  const unsigned long mode_button_settime_ms = 3000;
  const unsigned long mode_button_poweroff_ms = 5000;
  enum { INIT, PRE_GAME, IN_PROGRESS, PAUSED, SET_TIME, SET_TIME_CLOCK_1, SET_TIME_CLOCK_2 } game_state;
public:
  Player *players[NUM_PLAYERS];
  PollButton *mode_button;
  static int active_player;
  unsigned clock_being_set;
  unsigned long mode_button_last_pressed_at_ms;
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
    mode_button = new PollButton(A0);
    game_state = INIT;
    clock_being_set = 0;
    mode_button_last_pressed_at_ms = 0;
  }

  void init() {
    for(unsigned int i = 0; i < NUM_PLAYERS; i++) {
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
    for(unsigned int i = 0; i < NUM_PLAYERS; i++) {
      if (players[i]->button->interrupt_number == interrupt_number)
        return i;
    }
    return NONE;
  }

  void power_off() {
    //serprintf("Power off\r\n");
  }

  void pause() {
    if (game_state != IN_PROGRESS) {
      return;
    }
    serprintf("Pausing.\r\n");
    game_state = PAUSED;
  }

  void set_time() {
    if (game_state == SET_TIME) {
      return;
    }
    serprintf("Set time\r\n");
    game_state = SET_TIME;
    for(unsigned int i = 0; i < NUM_PLAYERS; i++) {
      players[i]->clock->init();
    }
  }

  void set_time_next_step() {
    players[clock_being_set]->update_display();
    clock_being_set++;
    if (clock_being_set >= NUM_PLAYERS) {
      clock_being_set = 0;
      serprintf("Done setting clocks, resetting\r\n");
      reset();
    }
  }

  void reset() {
    if (game_state == PRE_GAME) return;
    serprintf("Controller reset...\r\n");
    game_state = PRE_GAME;
    active_player = NONE;
    clock_being_set = 0;

    for(unsigned int i = 0; i < NUM_PLAYERS; i++) {
      serprintf("player %d clock...\r\n", i);
      players[i]->clock->init();
      serprintf("player %d screen...\r\n", i);
      players[i]->screen->reset();
      serprintf("player %d update_display...\r\n", i);
      players[i]->update_display();
    }
    serprintf("Controller reset complete.\r\n");
  }

  void handle_mode_button() {

    ButtonState *state = mode_button->poll();
    
    unsigned long press_duration_ms = state->press_duration_ms; 
    unsigned long pressed_at = state->pressed_at;
    bool button_pressed = state->pressed;
    bool just_released = state->just_released;
    bool new_button_press = false;
    if (pressed_at != mode_button_last_pressed_at_ms) {
      new_button_press = true;
      mode_button_last_pressed_at_ms = pressed_at;
    }
//    serprintf("bp: %d jr: %d dms: %lu gs: %d\r\n", button_pressed, just_released, press_duration_ms, game_state);
    
    switch (game_state) {
      case PRE_GAME:
        if (button_pressed) {
          if (press_duration_ms > mode_button_settime_ms) {
            set_time();
          } 
        }
        break;
      case IN_PROGRESS:
        if (button_pressed) {
          if (press_duration_ms > 0) {
            pause();
          }
        }
        break;
      case PAUSED:
        if (press_duration_ms > mode_button_reset_ms) {
          reset();
        }
        break;
      case SET_TIME:
        if (just_released) {
          game_state = SET_TIME_CLOCK_1;
        }
        break;
      case SET_TIME_CLOCK_1:
      case SET_TIME_CLOCK_2:
        if (press_duration_ms > 0 && new_button_press) {
          set_time_next_step();
        }
        break;
    }
  }


  void handle_pre_game_player_buttons() {
    if (interrupt_fired != NONE) {
      // Someone pressed a button since the last time we checked.
      int player_who_pressed = interrupt_to_player(interrupt_fired);

      // Unrecognized button interrupt
      if (player_who_pressed == NONE) {
        serprintf("received interrupt %d, but no player has a button on that interrupt!\r\n", interrupt_fired);
        return;
      }

      // A player is starting the game.
      if (active_player == NONE) {
        active_player = (player_who_pressed+1) % 2;
        game_state = IN_PROGRESS;
        players[active_player]->clock->start();
      }
      interrupt_fired = NONE;
    }
  }

  void handle_in_progress_player_buttons() {
    if (interrupt_fired != NONE) {
      // Someone pressed a button since the last time we checked.
      int player_who_pressed = interrupt_to_player(interrupt_fired);

      // Unrecognized button interrupt
      if (player_who_pressed == NONE) {
        serprintf("Received interrupt %d, but no player has a button on that interrupt!\r\n", interrupt_fired);
        return;
      }

      // the current player has hit their button, changing turns
      if (player_who_pressed == active_player) {
        serprintf("During the game, the active player hit thier button %d\r\n", player_who_pressed);
        active_player = (player_who_pressed+1) % 2;
        players[active_player]->clock->start();
      } else {
        //serprintf("Ignoring button press for inactive player %d\r\n", player_who_pressed);
      }
      interrupt_fired = NONE;
    }
  }

  void handle_set_time_player_buttons() {
    if (interrupt_fired != NONE) {
      int player_who_pressed = interrupt_to_player(interrupt_fired);

      // Unrecognized button interrupt
      if (player_who_pressed == NONE) {
        serprintf("Received interrupt %d, but no player has a button on that interrupt!\r\n", interrupt_fired);
        return;
      }

      serprintf("Setting the time, got player button %d clock being set:%d\r\n", player_who_pressed, clock_being_set);
      if (player_who_pressed == 0) {
        players[clock_being_set]->clock->add_minute();
      } else if (player_who_pressed == 1) {
        players[clock_being_set]->clock->subtract_minute();
      }
      interrupt_fired = NONE;
    }
  }

  void handle_paused_player_buttons() {
    if (interrupt_fired != NONE) {
      int player_who_pressed = interrupt_to_player(interrupt_fired);

      // Unrecognized button interrupt
      if (player_who_pressed == NONE) {
        serprintf("Received interrupt %d, but no player has a button on that interrupt!\r\n", interrupt_fired);
        return;
      }

      if (active_player == NONE) {
        serprintf("Supposed to pause, but there's no active palyer\r\n");
        return;
      }

      game_state = IN_PROGRESS;
      interrupt_fired = NONE;
    }
  }


  void tick() {
    unsigned long tick_ms = millis(); // the start of this tick
    switch (game_state) {
      case INIT:
        return;
        break;
      case PRE_GAME:
        handle_pre_game_player_buttons();
        break;
      case IN_PROGRESS:
        handle_in_progress_player_buttons();
        if (players[active_player]->out_of_time()) {
          players[active_player]->flag();
          break;
        } 
        players[active_player]->tick();
        break;
      case SET_TIME:
        break;
      case SET_TIME_CLOCK_1:
      case SET_TIME_CLOCK_2:
        handle_set_time_player_buttons();
        if (tick_ms % 1000 > 800) {
           players[clock_being_set]->blank_text();
        } else {
           players[clock_being_set]->update_display();
        }
        break;
      case PAUSED:
        handle_paused_player_buttons();
        break;
      default:
        serprintf("Should not get here; default case of controller tick switch!\r\n");
        break;
    }
    handle_mode_button();
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

