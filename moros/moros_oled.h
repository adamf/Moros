#ifndef __MOROS_OLED_H__
#define __MOROS_OLED_H__ 

//#include <fonts/Droid_Sans_24.h>
#include <fonts/Droid_Sans_36.h>

class Screen {
private:
  const unsigned margin_top = 20;
  const unsigned margin_left = 2;
  const unsigned margin_middle = 10;

public:
  OLED *oled;
  OLED_TextBox *box;
  Screen(unsigned int cs_pin, unsigned int dc_pin, unsigned int rst_pin) {
    oled = new OLED(cs_pin, dc_pin, rst_pin);
  };
  void init() {
    oled->begin();
    reset();
	oled->selectFont(Droid_Sans_36);
	box = new OLED_TextBox(*oled);
	box->setForegroundColor(WHITE);
  };

  void display_text(const char *text) {
	box->reset();
	box->println(text);
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
