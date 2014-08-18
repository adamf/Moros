#include <SPI.h>
#include <TFT.h>

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

void handle_button_press_1() {
  handle_button_press(1);
}
void handle_button_press_2() {
  handle_button_press(2);
}

typedef struct {
  unsigned int interrupt_number;
  void (*handler)();
} button;

button buttons[2] = {
  {
    .interrupt_number = 0,
    .handler = handle_button_press_1
  },
  {
    .interrupt_number = 1,
    .handler = handle_button_press_2
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



// does the SPI library allow selecting of which SS to issue the command on?

void setup(void) {
  Serial.begin(115200);
  Serial.println("Initializing...");

  char timea[12];
  for(unsigned int i = 0; i < sizeof(players) / sizeof(players[0]); i++) {
    players[i].tft.begin();
    players[i].tft.background(0,0,0);
    players[i].tft.stroke(255,255,255);
    players[i].tft.fill(0,0,0);
    players[i].tft.setTextSize(DISPLAY_FONT_SIZE);
//    Serial.println(players[i].tft.width());
    ltoa(players[i].time_remaining_ms/100, timea, 10);
    strncpy(players[i].display_time, timea, 12);
    players[i].tft.text(timea, 0, 20);
    attachInterrupt(players[i].interrupt_number, players[i].handle_button_press, RISING);
  }

  Serial.println("done.");
  //delay(100);
  //exit(1);
}


int loop_count = 0;
void loop() {
  char timea[12];
  if (loop_count == 0) {
    Serial.println("in loop");
    loop_count++;
  }
 // Serial.println(button_pressed);
  if (active_player != NONE) {
    if (players[active_player].time_remaining_ms <= 0) {
      active_player = NONE;
      button_pressed = NONE;
      return;
    }
    players[active_player].time_remaining_ms -= (millis() - players[active_player].last_update_ms);
    players[active_player].last_update_ms = millis();
    long display_time = players[active_player].time_remaining_ms/100;

    ltoa(display_time, timea, 10);
    for(unsigned int i = 0; i < strlen(players[active_player].display_time); i++) {
      if(timea[i] != players[active_player].display_time[i]) {
          
        // draw a rect
        players[active_player].tft.stroke(0,0,0);
        players[active_player].tft.rect(CHAR_WIDTH_PX * i , 20, CHAR_WIDTH_PX, CHAR_HEIGHT_PX);
        //players[active_player].tft.background(0,0,0);
        char timeb[2] = {timea[i], '\0' };
        players[active_player].tft.stroke(255,255,255);
        players[active_player].tft.text(timeb, CHAR_WIDTH_PX * i, 20);
      }
    }
    strncpy(players[active_player].display_time, timea, sizeof(players[active_player].display_time));
  }
  if ((active_player == NONE || button_pressed == active_player) && button_pressed != NONE)   {
    Serial.print("pre button_pressed: ");
    Serial.println(button_pressed);
    Serial.print("pre active_player: ");
    Serial.println(active_player);    
    active_player = (button_pressed + 1) % 2;
    players[active_player].last_update_ms = millis();
    Serial.print("post button_pressed: ");
    Serial.println(button_pressed);
    Serial.print("post active_player: ");
    Serial.println(active_player);    
  }     
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

}

