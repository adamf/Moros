// For the breakout, you can use any (4 or) 5 pins

#define cs_1   4
#define dc_1   5
#define rst_1  6 
#define cs_2   7
#define dc_2   8
#define rst_2  9
#define btn_1 2
#define btn_1_interrupt 0
#define btn_2 3
#define btn_2_interrupt 1

#include <SPI.h>
#include <TFT.h>

#define CLOCK_STOP -1

#define DISPLAY_FONT_SIZE 7
#define CHAR_HEIGHT_PX DISPLAY_FONT_SIZE * 10
#define CHAR_WIDTH_PX 42
#define DISPLAY_WIDTH_CHAR 4

//#define INITIAL_TIME_MS 300000
#define INITIAL_TIME_MS 300


int active_player = CLOCK_STOP;
volatile int button_pressed = CLOCK_STOP;

void handle_button_press_1() {
  button_pressed = 0;
}
void handle_button_press_2() {
  button_pressed = 1;
}

typedef struct {
  long time_remaining_ms;
  long last_update_ms;
  long btn_state;
  TFT tft;
  int button_pin;
  char display_time[12];
  int display_width_chars;
  int interrupt_number;
  void (* handle_button_press)();
} player;

#define PLAYER_COUNT 2
player players[PLAYER_COUNT] = { 
  {
    INITIAL_TIME_MS,
    0,
    0,
    TFT(cs_1, dc_1, rst_1),
    btn_1,
    "",
    0,
    btn_1_interrupt,
    handle_button_press_1
  },
  {
    INITIAL_TIME_MS,
    0,
    0,
    TFT(cs_2, dc_2, rst_2),
    btn_2,
    "",
    0,
    btn_2_interrupt,
    handle_button_press_2
  }
};



// does the SPI library allow selecting of which SS to issue the command on?

void setup(void) {
  Serial.begin(9600);
  Serial.println("hello!");

  char timea[12];
  for(int i = 0; i < PLAYER_COUNT; i++) {
    players[i].tft.begin();
    players[i].tft.background(0,0,0);
    players[i].tft.stroke(255,255,255);
    players[i].tft.fill(0,0,0);
    players[i].tft.setTextSize(DISPLAY_FONT_SIZE);
//    Serial.println(players[i].tft.width());
    ltoa(players[i].time_remaining_ms/100, timea, 10);
    strncpy(players[i].display_time, timea, 12);
    players[i].tft.text(timea, 0, 20);
//    pinMode(players[i].button_pin, INPUT);
    Serial.println("handle button press:");
    Serial.println((int)players[i].handle_button_press);
    Serial.println(players[i].interrupt_number);
    attachInterrupt(players[i].interrupt_number, players[i].handle_button_press, RISING);
  }

  Serial.println("init");
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
  if (active_player != CLOCK_STOP) {
    if (players[active_player].time_remaining_ms <= 0) {
      active_player = CLOCK_STOP;
      button_pressed = CLOCK_STOP;
      return;
    }
    players[active_player].time_remaining_ms -= (millis() - players[active_player].last_update_ms);
    players[active_player].last_update_ms = millis();
    long display_time = players[active_player].time_remaining_ms/100;

    ltoa(display_time, timea, 10);
    for(int i = 0; i < strlen(players[active_player].display_time); i++) {
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
  if ((active_player == CLOCK_STOP || button_pressed == active_player) && button_pressed != CLOCK_STOP)   {
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

