#ifndef __MOROS_OLED_H__
#define __MOROS_OLED_H__ 

#include <fonts/Droid_Sans_36.h>
#include "fonts/TulpenOneMono96.h"

class Screen {
private:
  const unsigned margin_top = 20;
  const unsigned margin_left = 2;
  const unsigned margin_middle = 10;

public:
  OLED *oled;
  OLED_TextBox *main_clock_box;
  OLED_TextBox *secondary_clock_box;
  Screen(unsigned int cs_pin, unsigned int dc_pin, unsigned int rst_pin) {
    oled = new OLED(cs_pin, dc_pin, rst_pin);
  };
  void init() {
    oled->begin();
    reset();
	oled->selectFont(Tulpen_One_Mono_96);
	main_clock_box = new OLED_TextBox(*oled, 12, 48, 108, 76);
	main_clock_box->setForegroundColor(WHITE);
	main_clock_box->setBackgroundColor(RED);

	secondary_clock_box = new OLED_TextBox(*oled, 88, 0, 36, 42);
	secondary_clock_box->setForegroundColor(WHITE);
	secondary_clock_box->setBackgroundColor(BLUE);

  };

  void display_text(const char *text) {
	oled->selectFont(Tulpen_One_Mono_96);
	main_clock_box->reset();
	main_clock_box->println(text);
  }

  void display_secondary_text(const char *text) {
	oled->selectFont(Droid_Sans_36);
	secondary_clock_box->reset();
	secondary_clock_box->println(text);
  }


  void change_text_size(int new_size) {
  }

  void display_flag() {
/*
    oled->stroke(0,0,255);
    oled->fill(0,0,255);
    oled->rect(margin_left, margin_top + fonts[FONT].char_height_px + margin_middle,
              oled->width() * 2 / 3, fonts[FONT].char_height_px);
*/
  }

  void reset() {
 //   oled->background(0,0,0);
 //   memset(prev_text, 0, sizeof(prev_text));
  }
};

#endif