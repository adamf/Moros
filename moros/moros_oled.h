#ifndef __MOROS_OLED_H__
#define __MOROS_OLED_H__ 

#include <fonts/Droid_Sans_36.h>
#include "fonts/TulpenOneMono96.h"
#include "cccp_flag.h"

class Screen {
public:
  OLED *oled;
  OLED_TextBox *main_clock_box;
  Screen(unsigned int cs_pin, unsigned int dc_pin, unsigned int rst_pin) {
    oled = new OLED(cs_pin, dc_pin, rst_pin);
  };
  void init() {
    oled->begin();
    reset();
	oled->selectFont(Tulpen_One_Mono_96);
	main_clock_box = new OLED_TextBox(*oled, 12, 48, 108, 76);
	main_clock_box->setForegroundColor(WHITE);
	main_clock_box->setBackgroundColor(BLACK);
  };

  void display_text(const char *text) {
	main_clock_box->setForegroundColor(WHITE);
	main_clock_box->reset();
	main_clock_box->println(text);
  }

  void change_text_size(int new_size) {
  }

  void display_flag() {
    oled->displayBMP(cccp_flag, 0, 0);
  }

  void reset() {
    oled->clearScreen();
  }

  void blank_text() {
	main_clock_box->clear();
  }
};

#endif
