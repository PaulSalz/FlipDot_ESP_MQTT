#include "arduino_secrets.h"
#include <Arduino.h>
#include <Bluepad32.h>
#include <vector>
int start_mode = 2;
int loop_state = start_mode;
int last_loop_state = start_mode;
bool try_wifi = true;
long tls_wifi = 0;
long tls_prt_dbg = 0;
long tls_font = 0;
long tls_case[7] = {0};
long tls_interval = 900;
bool b_tgl = false;


//--------------------------------------------------------------------------------------------------------------------------------
  #include <WiFi.h>
  #include <PubSubClient.h>
  #include <algorithm> 

  class Bitmap;  

  // ===== WLAN =====
  static const char* WIFI_SSID = SECRET_SSID;
  static const char* WIFI_PASS = SECRET_PASS;

  // ===== MQTT (Broker = Home Assistant / Mosquitto Add-on IP) =====
  static const char* MQTT_HOST = SECRET_HOST;
  static const uint16_t MQTT_PORT = 1883;
  static const char* MQTT_USER = SECRET_MUSER;       
  static const char* MQTT_PASS = SECRET_MPASS; 
  static const int mqtt_buf = 4096;

  // ===== Relais Pins =====
  static const int RELAY1_PIN = 19;
  static const bool RELAY_ACTIVE_LOW = true;
  bool relay1_On = false;
  static bool small_font_bool = false;

  // ===== Topics =====
  static const char* T_RELAY1_SET = "flipdot/relay1/set";
  static const char* T_RELAY1_STATE = "flipdot/relay1/state";
  static const char* T_RELAY2_SET = "flipdot/relay2/set";
  static const char* T_RELAY2_STATE = "flipdot/relay2/state";

  static const char* T_BTN_APPLY_CMD   = "flipdot/apply/press";
  static const char* T_BTN_180   = "flipdot/180/press";

  static const char* T_NUM_STATE_SET   = "flipdot/state/set";
  static const char* T_NUM_STATE_STATE = "flipdot/state/state";
  static int stateValue = start_mode;  // 0..10

  static const char* T_NUM_X_STATE_SET   = "flipdot/state_x/set";
  static const char* T_NUM_X_STATE_STATE = "flipdot/state_x/state";
  static int state_x = 0;  

  static const char* T_NUM_Y_STATE_SET   = "flipdot/state_y/set";
  static const char* T_NUM_Y_STATE_STATE = "flipdot/state_Y/state";
  static int state_y = 0;  

  static const char* T_TEXT_SET   = "flipdot/text/set";
  static const char* T_TEXT_STATE = "flipdot/text/state";
  static String textValue = "Hello World!";
  static bool textDirty = true;  

  static const char* AVAIL_TOPIC = "flipdot/availability";
  static const char* HA_STATUS_TOPIC = "homeassistant/status";
  static const char* DISCOVERY_PREFIX = "homeassistant";

  WiFiClient wifiClient;
  PubSubClient mqtt(wifiClient);

  bool isEven(long n){
    return n % 2 == 0;
  }
  static void applyRelay(int pin, bool on) {
    on = !on;
    if (RELAY_ACTIVE_LOW) digitalWrite(pin, on ? LOW : HIGH);
    else digitalWrite(pin, on ? HIGH : LOW);
  }
  static void publishStateControls(bool retain=true) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", stateValue);
    mqtt.publish(T_NUM_STATE_STATE, buf, retain);
  }
  static void publishState_xy(bool retain=true) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", state_x);
    mqtt.publish(T_NUM_X_STATE_STATE, buf, retain);

    snprintf(buf, sizeof(buf), "%d", state_y);
    mqtt.publish(T_NUM_Y_STATE_STATE, buf, retain);
  }
  static void publishStates(bool retain = true) {
    mqtt.publish(T_RELAY1_STATE, relay1_On ? "ON" : "OFF", retain);
    mqtt.publish(T_RELAY2_STATE, small_font_bool ? "ON" : "OFF", retain);
  }
  static void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String t(topic);

    String msg;
    msg.reserve(length);
    for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

    if (t == T_TEXT_SET) {
      // NICHT trimmen, sonst verschwinden Leerzeichen am Anfang/Ende
      textValue = msg;
      // HA text max = 255 (optional clamp)
      if (textValue.length() > 255) textValue = textValue.substring(0, 255);
      textDirty = true;
      // State zurückmelden (retained)
      mqtt.publish(T_TEXT_STATE, textValue.c_str(), true);
      return;
    }
    msg.trim();
    if (t == HA_STATUS_TOPIC) {
      if (msg == "online") {
        publishDiscovery();
        publishStates(true);
      }
      return;
    }
    bool on;
    if (msg == "ON" || msg == "1" || msg == "true") on = true;
    else if (msg == "OFF" || msg == "0" || msg == "false") on = false;

    if (t == T_RELAY1_SET) {
      relay1_On = on;
      applyRelay(RELAY1_PIN, relay1_On);
      mqtt.publish(T_RELAY1_STATE, relay1_On ? "ON" : "OFF", true);
      textDirty = true;
    }
    else if (t == T_RELAY2_SET) {  
      small_font_bool = on;
      textDirty = true;
      mqtt.publish(T_RELAY2_STATE, small_font_bool ? "ON" : "OFF", true);
    }
    else if (t == T_BTN_APPLY_CMD) { // Button: Apply 
      //Serial.printf("Apply pressed\n");
      ESP.restart();
      return;
    }
    else if (t == T_BTN_180) { // Button
      //Serial.printf("180 pressed\n");
      last_loop_state = loop_state;
      loop_state = 180;
      textDirty = true;
      return;
    }
    else if (t == T_NUM_STATE_SET) {// Number: State set
      int v = msg.toInt();           
      if (v < 0) v = 0;
      if (v > 10) v = 10;
      stateValue = v;
      loop_state = stateValue;
      publishStateControls(true);
      //Serial.printf("loop_state=%d\n", loop_state);
      tls_prt_dbg = millis();
      textDirty = true;
      return;
    }
    else if (t == T_NUM_X_STATE_SET) {// X
      int v = msg.toInt();           
      if (v < 0) v = 0;
      if (v > 112) v = 112;
      state_x = v;
      publishState_xy(true);
      textDirty = true;
      return;
    }
    else if (t == T_NUM_Y_STATE_SET) {// Y 
      int v = msg.toInt();           
      if (v < 0) v = 0;
      if (v > 16) v = 16;
      state_y = v;
      publishState_xy(true);
      textDirty = true;
      return;
    }     

  }
  static void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    tls_wifi = millis();
    try_wifi = true;

    while (WiFi.status() != WL_CONNECTED  && try_wifi) {
      //Serial.printf("Connecting...\n");
      delay(250);
      if(millis() - tls_wifi > 20000){
        try_wifi = false;
      }
    }
  }
  static void ensureMqtt() {
    while (!mqtt.connected()) {
      Serial.printf("mqtt Connecting...\n");

      String cid = "flipdot-esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
      bool ok = false;
      if (MQTT_USER && MQTT_PASS && MQTT_USER[0] != '\0') {
        ok = mqtt.connect(
          cid.c_str(),
          MQTT_USER, MQTT_PASS,
          AVAIL_TOPIC, 0, true, "offline"  // willTopic, willQoS, willRetain, willMessage
        );
      } 
      else {
        ok = mqtt.connect(
          cid.c_str(),
          AVAIL_TOPIC, 0, true, "offline"  
        );
      }

      if (ok) {
        mqtt.setBufferSize(mqtt_buf);
        mqtt.subscribe(T_BTN_APPLY_CMD);
        mqtt.subscribe(T_BTN_180);
        mqtt.subscribe(T_NUM_STATE_SET);
        mqtt.subscribe(T_NUM_X_STATE_SET);
        mqtt.subscribe(T_NUM_Y_STATE_SET);
        publishStateControls(true);
        publishState_xy(true);
        mqtt.subscribe(T_TEXT_SET);
        mqtt.publish(T_TEXT_STATE, textValue.c_str(), true);
        mqtt.subscribe(T_RELAY1_SET);
        mqtt.subscribe(T_RELAY2_SET);
        mqtt.subscribe(HA_STATUS_TOPIC);
        mqtt.publish(AVAIL_TOPIC, "online", true);
        publishDiscovery();
        publishStates(true);
        Serial.printf("mqtt Connected.\n");
      } 
      else {
        Serial.printf("mqtt connect failed, rc=%d\n", mqtt.state());
        delay(2500);
      }
    }
  }
  static String devId() {
    return "flipdot_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  }
  static void publishDiscovery() {
    String id = devId();

    String t1 = String(DISCOVERY_PREFIX) + "/switch/" + id + "/relay1/config";
    String t2 = String(DISCOVERY_PREFIX) + "/switch/" + id + "/relay2/config";

    String devObj =
      String("\"device\":{") + "\"identifiers\":[\"" + id + "\"]," + "\"name\":\"Flipdot Controller\"," + "\"manufacturer\":\"DIY\"," + "\"model\":\"ESP32 + Mobitec\"," + "\"sw_version\":\"1.0\"" + "}";

    String p1 =
      String("{") + "\"name\":\"LIGHTS\"," + "\"unique_id\":\"" + id + "_relay1\"," + "\"command_topic\":\"" + String(T_RELAY1_SET) + "\"," + "\"state_topic\":\"" + String(T_RELAY1_STATE) + "\"," + "\"payload_on\":\"ON\",\"payload_off\":\"OFF\"," + "\"state_on\":\"ON\",\"state_off\":\"OFF\"," + "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," + "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," + devObj + "}";

    String p2 =
      String("{") + "\"name\":\"Small FONT\"," + "\"unique_id\":\"" + id + "_relay2\"," + "\"command_topic\":\"" + String(T_RELAY2_SET) + "\"," + "\"state_topic\":\"" + String(T_RELAY2_STATE) + "\"," + "\"payload_on\":\"ON\",\"payload_off\":\"OFF\"," + "\"state_on\":\"ON\",\"state_off\":\"OFF\"," + "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," + "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," + devObj + "}";

    bool ok1 = mqtt.publish(t1.c_str(), p1.c_str(), true);
    bool ok2 = mqtt.publish(t2.c_str(), p2.c_str(), true);

    String tb = String(DISCOVERY_PREFIX) + "/button/" + id + "/apply/config";
    String tb_180 = String(DISCOVERY_PREFIX) + "/button/" + id + "/180/config";

    String tn = String(DISCOVERY_PREFIX) + "/number/" + id + "/state/config";
    String tx = String(DISCOVERY_PREFIX) + "/number/" + id + "/state_x/config";
    String ty = String(DISCOVERY_PREFIX) + "/number/" + id + "/state_y/config";

    String pb =
      String("{") +
      "\"name\":\"Restart\"," +
      "\"unique_id\":\"" + id + "_apply\"," +
      "\"command_topic\":\"" + String(T_BTN_APPLY_CMD) + "\"," +
      "\"payload_press\":\"PRESS\"," +
      "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
      "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
      devObj +
      "}";
    String pb_180 =
      String("{") +
      "\"name\":\"180\"," +
      "\"unique_id\":\"" + id + "_180\"," +
      "\"command_topic\":\"" + String(T_BTN_180) + "\"," +
      "\"payload_press\":\"PRESS\"," +
      "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
      "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
      devObj +
      "}";
    String pn =
      String("{") +
      "\"name\":\"MODE\"," +
      "\"unique_id\":\"" + id + "_state\"," +
      "\"command_topic\":\"" + String(T_NUM_STATE_SET) + "\"," +
      "\"state_topic\":\"" + String(T_NUM_STATE_STATE) + "\"," +
      "\"min\":0,\"max\":10,\"step\":1," +
      "\"mode\":\"box\"," +
      "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
      "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
      devObj +
      "}";
    String px =
      String("{") +
      "\"name\":\"Pos_x\"," +
      "\"unique_id\":\"" + id + "_state_x\"," +
      "\"command_topic\":\"" + String(T_NUM_X_STATE_SET) + "\"," +
      "\"state_topic\":\"" + String(T_NUM_X_STATE_STATE) + "\"," +
      "\"min\":0,\"max\":112,\"step\":1," +
      "\"mode\":\"box\"," +
      "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
      "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
      devObj +
      "}";
    String py =
      String("{") +
      "\"name\":\"Pos_y\"," +
      "\"unique_id\":\"" + id + "_state_y\"," +
      "\"command_topic\":\"" + String(T_NUM_Y_STATE_SET) + "\"," +
      "\"state_topic\":\"" + String(T_NUM_Y_STATE_STATE) + "\"," +
      "\"min\":0,\"max\":16,\"step\":1," +
      "\"mode\":\"box\"," +
      "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
      "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
      devObj +
      "}";  

    
    mqtt.publish(tb.c_str(), pb.c_str(), true);
    mqtt.publish(tb_180.c_str(), pb_180.c_str(), true);
    mqtt.publish(tn.c_str(), pn.c_str(), true);
    mqtt.publish(tx.c_str(), px.c_str(), true);
    mqtt.publish(ty.c_str(), py.c_str(), true);

    String tt = String(DISCOVERY_PREFIX) + "/text/" + id + "/display/config";
    String pt =
      String("{") +
      "\"name\":\"Display Text\"," +
      "\"unique_id\":\"" + id + "_display_text\"," +
      "\"command_topic\":\"" + String(T_TEXT_SET) + "\"," +
      "\"state_topic\":\"" + String(T_TEXT_STATE) + "\"," +
      "\"mode\":\"text\"," +
      "\"min\":0,\"max\":64," +   // 64 z.B. sinnvoll fürs Flipdot; max in HA ist 255
      "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
      "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
      devObj +
      "}";

    mqtt.publish(tt.c_str(), pt.c_str(), true);


  }

//--------------------------------------------------------------------------------------------------------------------------------
  using std::vector;

  // ---------------- Fonts ----------------
  struct Font {
    const char* name;
    uint8_t height;
    uint8_t code;
    bool isPixelSubcolumns;
  };

  static const Font FONT_7PX = { "7px", 7, 0x60, false };
  static const Font FONT_7PX_WIDE = { "7px_wide", 7, 0x62, false };
  static const Font FONT_12PX = { "12px", 12, 0x63, false };
  static const Font FONT_13PX = { "13px", 13, 0x64, false };
  static const Font FONT_13PX_WIDE = { "13px_wide", 13, 0x65, false };
  static const Font FONT_13PX_WIDER = { "13px_wider", 13, 0x69, false };
  static const Font FONT_16PX_NUMBERS = { "16px_numbers", 16, 0x68, false };
  static const Font FONT_16PX_NUM_WIDE = { "16px_numbers_wide", 16, 0x6a, false };
  static const Font FONT_PIXEL_SUBCOL = { "pixel_subcolumns", 5, 0x77, true };

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

      // UTF-8 Mapping für ÅåÄäÖö (U+00C5/U+00E5/U+00C4/U+00E4/U+00D6/U+00F6)
      const uint8_t* p = (const uint8_t*)t.s.c_str();
      for (size_t i = 0; p[i] != 0; i++) {
        uint8_t c = p[i];
        if (c < 0x80) {
          // ASCII direkt
          out.push_back(c);
        } else if (c == 0xC3 && p[i + 1] != 0) {
          uint8_t d = p[i + 1];
          // Å: C3 85 -> 0x5D
          if (d == 0x85) out.push_back(0x5D);
          // å: C3 A5 -> 0x7D
          else if (d == 0xA5) out.push_back(0x7D);
          // Ä: C3 84 -> 0x5B
          else if (d == 0x84) out.push_back(0x5B);
          // ä: C3 A4 -> 0x7B
          else if (d == 0xA4) out.push_back(0x7B);
          // Ö: C3 96 -> 0x5C
          else if (d == 0x96) out.push_back(0x5C);
          // ö: C3 B6 -> 0x7C
          else if (d == 0xB6) out.push_back(0x7C);
          i++; 
        } else {

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
  static const int UART_RX_PIN = 16;
  static const int UART_TX_PIN = 17;

  // Display 
  static const uint8_t DISP_ADDR = 0x06;
  static const uint8_t DISP_W = 112;
  static const uint8_t DISP_H = 16;

  // UART 
  HardwareSerial& RS485 = Serial2;
  MobitecDisplay flipdot(RS485, DISP_ADDR, DISP_W, DISP_H);

  // Bitmaps
  Bitmap bmFull(112, 16, 0, 0);
  Bitmap bm_tree(112, 16, 0, 0);

  static void drawTree3LevelsInto(Bitmap& target, int dstX, int dstY) {
    static const uint16_t rows[16] = {
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
    flipdot.setPosition(state_x, state_y);
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


//--------------------------------------------------------------------------------------------------------------------------------

  GamepadPtr pads[BP32_MAX_GAMEPADS];

  static void onConnectedGamepad(GamepadPtr gp) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
      if (!pads[i]) { pads[i] = gp; break; }
    }
    Serial.println("PS4 connected");
  }

  static void onDisconnectedGamepad(GamepadPtr gp) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
      if (pads[i] == gp) { pads[i] = nullptr; break; }
    }
    Serial.println("PS4 disconnected");
  }
  static bool prevA=false, prevB=false, prevX=false, prevY=false;

  void handle_controller(){
    BP32.update();
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
      GamepadPtr gp = pads[i];
      if (!gp || !gp->isConnected()) continue;
      bool A = gp->a();   
      bool B = gp->b();   
      bool X = gp->x();   
      bool Y = gp->y();   
      if (B && !prevB) {
        if(try_wifi){
          mqtt.publish(T_BTN_180, "PRESS", false);
        }
        else{
          last_loop_state = loop_state;
          loop_state = 180;
          textDirty = true;
        }
        Serial.println("PS4: B press");
      }
      if (X && !prevX) {
        if(try_wifi){
          //mqtt.publish(T_BTN_180, "PRESS", false);
        }
        loop_state=1;
        Serial.println("PS4: X press");
      }
      if (A && !prevA) {
        relay1_On = !relay1_On;
        if(try_wifi){
          mqtt.publish(T_RELAY1_SET, relay1_On ? "ON" : "OFF", false);
        }
        else{
          applyRelay(RELAY1_PIN, relay1_On);
        }
        Serial.println("PS4: A");
      }
      if (Y && !prevY) {
        loop_state++;
        if (loop_state > 6){
          loop_state = 1;
        }
        if(try_wifi){
          publishStateControls(true);
        }
        Serial.printf("PS4: mode=%d\n", stateValue);
      }
      prevA = A; prevB = B; prevX = X; prevY = Y;
    }
  }


//--------------------------------------------------------------------------------------------------------------------------------

  void setup() {
    drawBMfull(bmFull);
    Serial.begin(115200);
    delay(1500);
    flipdot.begin(4800, UART_RX_PIN, UART_TX_PIN);
    Serial.printf("\n\nHello World\n");
    pinMode(RELAY1_PIN, OUTPUT);
    applyRelay(RELAY1_PIN, relay1_On);

    single_msg("connecting . . .");
    Serial.printf("Starting WIFI\n");
    if(try_wifi){
      connectWiFi();
      Serial.println(WiFi.localIP());
      Serial.println(WiFi.gatewayIP());
      Serial.println(WiFi.status());
      Serial.printf("WIFI done\n");
      single_msg("Wifi done");
      delay(500);
      single_msg("MQTT . . .");
      mqtt.setServer(MQTT_HOST, MQTT_PORT);
      mqtt.setBufferSize(mqtt_buf);
      mqtt.setCallback(mqttCallback);
      ensureMqtt();
      Serial.printf("MQTT done\n");
      single_msg("CONNECTED");
      delay(1500);
    }
    else{
      Serial.printf("Wifi not connected\n");
      single_msg("not connected");
      delay(1500);
      Serial.printf("Enable BT\n");
      BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
      BP32.enableVirtualDevice(false);   
      single_msg("BT ready");
      Serial.printf("Enable BT\n");
      delay(1500);
    }    
    flipdot.clearBuffer();
    flipdot.show();
    delay(1500);
    Serial.printf("Setup done\n");
  }


//--------------------------------------------------------------------------------------------------------------------------------
  


  void loop() {
    if(try_wifi){
      ensureMqtt();
      mqtt.loop();
    }
    else{
      handle_controller();
    }

    //FONT set
    if(millis()-tls_font > 500){
      if(small_font_bool){
        flipdot.setFont(FONT_7PX_WIDE);
      }
      else{
        flipdot.setFont(FONT_7PX);
      }
    }

    switch (loop_state) {
      case 1:// hi blinkend
        if(millis()-tls_case[loop_state] > tls_interval){
          tls_case[loop_state] = millis();
          //Serial.printf("Hi, %d\n", b_tgl);          
          if(b_tgl) {
            flipdot.setPosition(state_x, state_y);
            flipdot.printText("Hi");
            flipdot.show();
          }
          else{
            flipdot.clearBuffer();
            flipdot.show();
          }
          b_tgl = !b_tgl;
        }
        break;

      case 2:// text single
        if(millis()-tls_case[loop_state] > tls_interval){
          tls_case[loop_state] = millis();    
          if (textDirty) {
            textDirty = false;
            flipdot.clearBuffer();
            flipdot.setPosition(state_x, state_y);
            flipdot.printText(textValue);
            flipdot.show();
          }
        }
        break;  
      case 3://text blinkend
        if(millis()-tls_case[loop_state] > tls_interval+500){
          tls_case[loop_state] = millis();
          if(b_tgl) {
            flipdot.clearBuffer();
            flipdot.setPosition(state_x, state_y);
            flipdot.printText(textValue);
            flipdot.show();
          }
          else{
            flipdot.clearBuffer();
            flipdot.show();
          }
          b_tgl = !b_tgl;
        }
        break;

      case 4: // Xmas und Bäume
        if(millis()-tls_case[loop_state] > tls_interval+20000){
          tls_case[loop_state] = millis();      
          if(b_tgl) {
            flipdot.clearBuffer();
            bm_tree.clear();
            for(int i=3; i<103; i+=12){
              drawTree3LevelsInto(bm_tree, i, 0);
            }
            flipdot.printBitmap(bm_tree);
            flipdot.show();
          }
          else{
            flipdot.clearBuffer();
            flipdot.setPosition(5, 9);
            flipdot.printText("Merry Xmas!");
            flipdot.show();
          }
          b_tgl = !b_tgl;
        }
        break;

      case 5://Full on
        if(millis()-tls_case[loop_state] > tls_interval){
          flipdot.clearBuffer();
          tls_case[loop_state] = millis();
          flipdot.printBitmap(bmFull);
          flipdot.show();
        }
        break;  
      case 180: //Darts
        int i;
        i = 10;
        while (i > 0){
          if(millis()-tls_case[7] > 500){
            tls_case[7] = millis();    
            if(isEven(i)) {
              flipdot.clearBuffer();
              flipdot.setPosition(8, 9);
              flipdot.setFont(FONT_7PX_WIDE);
              flipdot.printText("   !!!  180  !!!");
              flipdot.show();
            }
            else{
              flipdot.clearBuffer();
              flipdot.setPosition(3, 9);
              flipdot.setFont(FONT_7PX);
              flipdot.printText("  !!!   180  !!!");
              flipdot.show();
            }
            i--;
          }
        }
        loop_state = last_loop_state;
        break;
      default:
        if(millis()-tls_prt_dbg > 1000){
          Serial.printf("ENDE!  loop_state: %d\n", loop_state);
          tls_prt_dbg = millis();
          single_msg("mode!");
        }
        break;
    }
  }

































