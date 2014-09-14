#ifndef __MOROS_TFT_H__
#define __MOROS_TFT_H__ 

struct TFTFont {
  unsigned int font_size;
  unsigned int char_height_px;
  unsigned int char_width_px;
};

TFTFont fonts[4] = {
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
  const unsigned margin_top = 20;
  const unsigned margin_left = 2;
  const unsigned margin_middle = 10;

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
        tft->fill(0,0,0);
        // tft->stroke(255,0,0); // draw bounding box
        tft->stroke(0,0,0);
        tft->rect(margin_left + fonts[FONT].char_width_px * i, 20,
                  fonts[FONT].char_width_px, fonts[FONT].char_height_px);

        // print the new character
        char next_char[2] = {text[i], '\0' };
        tft->stroke(255,255,255);
        tft->text(next_char, margin_left + i * fonts[FONT].char_width_px, margin_top);
      }
    }
    strncpy(prev_text, text, sizeof(prev_text));
  }

  void display_flag() {
    tft->stroke(0,0,255);
    tft->fill(0,0,255);
    tft->rect(margin_left, margin_top + fonts[FONT].char_height_px + margin_middle,
              tft->width() * 2 / 3, fonts[FONT].char_height_px);
  }
};

#endif
