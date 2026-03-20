#pragma once

using std::vector;

// ---------------- Fonts ----------------
struct Font {
  const char* name;
  uint8_t height;
  uint8_t code;
  bool isPixelSubcolumns;
};

  const Font FONT_7PX = { "7px", 7, 0x60, false };
  const Font FONT_7PX_WIDE = { "7px_wide", 7, 0x62, false };
  const Font FONT_12PX = { "12px", 12, 0x63, false };
  const Font FONT_13PX = { "13px", 13, 0x64, false };
  const Font FONT_13PX_WIDE = { "13px_wide", 13, 0x65, false };
  const Font FONT_13PX_WIDER = { "13px_wider", 13, 0x69, false };
  const Font FONT_16PX_NUMBERS = { "16px_numbers", 16, 0x68, false };
  const Font FONT_16PX_NUM_WIDE = { "16px_numbers_wide", 16, 0x6a, false };
  const Font FONT_PIXEL_SUBCOL = { "pixel_subcolumns", 5, 0x77, true };

// ---------------- Bitmap ----------------
class Bitmap {
public:
  uint8_t width, height;
  uint8_t pos_x, pos_y;

  Bitmap(uint8_t w, uint8_t h, uint8_t x, uint8_t y)
    : width(w), height(h), pos_x(x), pos_y(y), pix((size_t)w * (size_t)h, 0) {}

  void clear() {
    std::fill(pix.begin(), pix.end(), 0);
  }

  void fill(uint8_t v) {
    std::fill(pix.begin(), pix.end(), v ? 1 : 0);
  }

  void setPixel(int x, int y, bool on) {
    if (x < 0 || y < 0 || x >= width || y >= height) return;
    pix[(size_t)y * (size_t)width + (size_t)x] = on ? 1 : 0;
  }

  bool getPixel(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) return false;
    return pix[(size_t)y * (size_t)width + (size_t)x] != 0;
  }

  uint8_t subcolumnCode(uint8_t band, uint8_t x) const {
    uint8_t ret = 32;
    uint8_t baseY = (uint8_t)(band * 5);
    for (uint8_t i = 0; i < 5; i++) {
      uint8_t yy = baseY + i;
      if (yy >= height) break;
      if (getPixel(x, yy)) ret = (uint8_t)(ret + (1u << i));
    }
    return ret;
  }

  uint8_t bandCount() const {
    return (uint8_t)((height + 4) / 5);
  }

private:
  vector<uint8_t> pix;  // 0/1 per Pixel (112*16 = 1792 Byte)
};

// ---------------- Text ----------------
struct TextItem {
  String s;
  const Font* font;
  uint8_t x, y;
};

// ---------------- Mobitec Display ----------------
class MobitecDisplay {
public:
  MobitecDisplay(HardwareSerial& uart,
                uint8_t address,
                uint8_t width,
                uint8_t height)
    : ser(uart), addr(address), w(width), h(height) {
    curFont = &FONT_7PX;
    posX = 0;
    posY = 0;
  }

  void begin(uint32_t baud, int rxPin, int txPin) {
    ser.begin(baud, SERIAL_8N1, rxPin, txPin);
    pinMode(dePin, OUTPUT);
    digitalWrite(dePin, LOW); 
  }

  void clearBuffer() {
    texts.clear();
    bitmaps.clear();
  }

  void setPosition(uint8_t x, uint8_t y) {
    posX = x;
    posY = y;
  }

  void setFont(const Font& f) {
    curFont = &f;
  }

  void printText(const String& s) {
    TextItem t;
    t.s = s;
    t.font = curFont;
    t.x = posX;
    t.y = posY;
    texts.push_back(t);
  }

  void printBitmap(const Bitmap& bm) {
    bitmaps.push_back(&bm);
  }

  void drawPixel(uint8_t x, uint8_t y) {
    const Font* saved = curFont;
    setFont(FONT_PIXEL_SUBCOL);
    setPosition(x, y);
    printText("!");
    curFont = saved;
  }

  void show() {
    vector<uint8_t> packet;
    packet.reserve(1024);

    packet.push_back(0xFF);  // Start
    appendPacketHeader(packet);

    for (const auto& t : texts) appendText(packet, t);
    for (const auto* bm : bitmaps) appendBitmap(packet, *bm);

    appendChecksum(packet);
    packet.push_back(0xFF);

    // RX leeren (wie reset_input_buffer)
    while (ser.available()) ser.read();
    ser.write(packet.data(), packet.size());
    ser.flush();
  }

private:
  HardwareSerial& ser;
  int dePin;
  uint8_t addr, w, h;

  const Font* curFont;
  uint8_t posX, posY;

  vector<TextItem> texts;
  vector<const Bitmap*> bitmaps;

  void appendPacketHeader(vector<uint8_t>& out) {
    out.push_back(addr);
    out.push_back(0xA2);  
    out.push_back(0xD0);
    out.push_back(w);
    out.push_back(0xD1);
    out.push_back(h);
  }

  void appendDataHeader(vector<uint8_t>& out, uint8_t fontCode, uint8_t x, uint8_t y) {
    out.push_back(0xD2);
    out.push_back(x);
    out.push_back(0xD3);
    out.push_back(y);
    out.push_back(0xD4);
    out.push_back(fontCode);
  }

  void appendText(vector<uint8_t>& out, const TextItem& t) {
    uint8_t y = (uint8_t)(t.y + t.font->height);
    if (t.font->isPixelSubcolumns && y > 0) y = (uint8_t)(y - 1);

    appendDataHeader(out, t.font->code, t.x, y);

    const uint8_t* p = (const uint8_t*)t.s.c_str();
    for (size_t i = 0; p[i] != 0; i++) {
      uint8_t c = p[i];

      if (c < 0x80) {
        if (c == ',') c = '.'; 
        out.push_back(c);
      }
      else if (c == 0xC3 && p[i + 1] != 0) {
        uint8_t d = p[i + 1];
        if (d == 0x85) out.push_back(0x5D);      // Å
        else if (d == 0xA5) out.push_back(0x7D); // å
        else if (d == 0x84) out.push_back(0x5B); // Ä
        else if (d == 0xA4) out.push_back(0x7B); // ä
        else if (d == 0x96) out.push_back(0x5C); // Ö
        else if (d == 0xB6) out.push_back(0x7C); // ö
        i++;
      } else {
        // unbekannt -> ignorieren
      }
    }
  }


  void appendBitmap(vector<uint8_t>& out, const Bitmap& bm) {
    uint8_t bands = bm.bandCount();
    for (uint8_t band = 0; band < bands; band++) {
      uint8_t y = (uint8_t)(bm.pos_y + (uint8_t)(band * 5) + 4); 
      appendDataHeader(out, 0x77, bm.pos_x, y);
      for (uint8_t x = 0; x < bm.width; x++) {
        out.push_back(bm.subcolumnCode(band, x));
      }
    }
  }

  void appendChecksum(vector<uint8_t>& out) {
    uint16_t sum = 0;
    for (size_t i = 1; i < out.size(); i++) sum = (uint16_t)(sum + out[i]);
    uint8_t cs = (uint8_t)(sum & 0xFF);

    if (cs == 0xFE) {
      out.push_back(0xFE);
      out.push_back(0x00);
    } else if (cs == 0xFF) {
      out.push_back(0xFE);
      out.push_back(0x01);
    } else out.push_back(cs);
  }
};

// ---------------- User Settings ----------------
// UART2 Default-Pins 
  const int UART_RX_PIN = 16;
  const int UART_TX_PIN = 17;

// Display 
  const uint8_t DISP_ADDR = 0x06;
  const uint8_t DISP_W = 112;
  const uint8_t DISP_H = 16;

// UART 
HardwareSerial& RS485 = Serial2;
MobitecDisplay flipdot(RS485, DISP_ADDR, DISP_W, DISP_H);

// Bitmaps
Bitmap bmFull(112, 16, 0, 0);
Bitmap bm_tree(112, 16, 0, 0);

  void drawTree3LevelsInto(Bitmap& target, int dstX, int dstY) {
    const uint16_t rows[16] = {
    0b000010000, 
    0b000111000, 
    0b001111100, 
    0b000000000,
    0b000111000, 
    0b001111100, 
    0b011111110, 
    0b111111111, 
    0b000000000,
    0b001111100, 
    0b011111110, 
    0b111111111, 
    0b111111111, 
    0b111111111,
    0b000111000, 
    0b000111000
  };

  for (int yy = 0; yy < 16; yy++) {
    uint16_t mask = rows[yy];
    for (int xx = 0; xx < 9; xx++) {
      if (mask & (1u << xx)) {
        target.setPixel(dstX + xx, dstY + yy, true);
      }
    }
  }
}
void single_msg(String txt){
  flipdot.clearBuffer();
  flipdot.setPosition(0, 0);
  flipdot.setFont(FONT_7PX_WIDE);
  flipdot.printText(txt);
  flipdot.show();
}
void drawBMfull(Bitmap& target){
  for (int yy = 0; yy < 16; yy++) {
    for (int xx = 0; xx < 112; xx++) {
        target.setPixel(xx, yy, true);
    }
  }
}

void switch_font(int font_var){
  switch(font_var){
        case 0: // smallest 7 high
            flipdot.setFont(FONT_13PX_WIDE);
          break;
        case 1: // 7 high thick
            flipdot.setFont(FONT_13PX);
          break;
        case 2: // middle 9 high
            flipdot.setFont(FONT_7PX_WIDE);
          break;
        case 3: // big 13 high
            flipdot.setFont(FONT_13PX_WIDER);
          break;
        case 4: // 13 high thick
            flipdot.setFont(FONT_7PX);
          break;
        case 5: // 16px high
            flipdot.setFont(FONT_16PX_NUMBERS);
          break;
        case 6: //way too big
            flipdot.setFont(FONT_16PX_NUM_WIDE);
          break;
        case 7: // way too big
            flipdot.setFont(FONT_12PX);
          break;
        case 8: // Subpixel
            flipdot.setFont(FONT_PIXEL_SUBCOL);
          break;
        default:
            flipdot.setFont(FONT_7PX);
          break;
      }
}








