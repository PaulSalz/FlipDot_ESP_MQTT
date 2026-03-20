#include "b_controller.h"
#pragma once
void handleTimeSync();
void handle_controller();

class Bitmap;  

//CONFIG
// ============================================================================
  // Network configuration

    const char* WIFI_SSID = SECRET_SSID;
    const char* WIFI_PASS = SECRET_PASS;

    const char* MQTT_HOST = SECRET_HOST;
    const uint16_t MQTT_PORT = 1883;
    const char* MQTT_USER = SECRET_MUSER;
    const char* MQTT_PASS = SECRET_MPASS;
    const int MQTT_BUFFER_SIZE = 8192;

  // Hardware configuration

    const int RELAY1_PIN = 19;
    const bool RELAY_ACTIVE_LOW = true;

  // Home Assistant / MQTT base topics

    const char* TOPIC_BASE = "flipdot";
    const char* AVAIL_TOPIC = "flipdot/availability";
    const char* HA_STATUS_TOPIC = "homeassistant/status";
    const char* DISCOVERY_PREFIX = "homeassistant";

  // MQTT command/state topics
  // Relay
    const char* T_RELAY1_SET   = "flipdot/relay1/set";
    const char* T_RELAY1_STATE = "flipdot/relay1/state";

  // Time-to-text switch
    const char* T_SW_TTT_SET   = "flipdot/sw_ttt/set";
    const char* T_SW_TTT_STATE = "flipdot/sw_ttt/state";

  // Display Seconds switch
    const char* T_SW_SHOWSEC_SET   = "flipdot/sw_showSeconds/set";
    const char* T_SW_SHOWSEC_STATE = "flipdot/sw_showSeconds/state";

  // Buttons
    const char* T_BTN_RESTART = "flipdot/restart/press";
    const char* T_BTN_180       = "flipdot/180/press";

  // Row 1 position
    const char* T_NUM_X_STATE_SET   = "flipdot/state_x/set";
    const char* T_NUM_X_STATE_STATE = "flipdot/state_x/state";

    const char* T_NUM_Y_STATE_SET   = "flipdot/state_y/set";
    const char* T_NUM_Y_STATE_STATE = "flipdot/state_y/state";

  // Row 2 position
    const char* T_NUM_X2_STATE_SET   = "flipdot/state_x2/set";
    const char* T_NUM_X2_STATE_STATE = "flipdot/state_x2/state";

    const char* T_NUM_Y2_STATE_SET   = "flipdot/state_y2/set";
    const char* T_NUM_Y2_STATE_STATE = "flipdot/state_y2/state";

  // General numeric settings
    const char* T_NUM_RATE_STATE_SET   = "flipdot/state_updateRate/set";
    const char* T_NUM_RATE_STATE_STATE = "flipdot/state_updateRate/state";

    const char* T_NUM_FONT_STATE_SET   = "flipdot/state_font/set";
    const char* T_NUM_FONT_STATE_STATE = "flipdot/state_font/state";

    const char* T_NUM_FONT2_STATE_SET   = "flipdot/state_font2/set";
    const char* T_NUM_FONT2_STATE_STATE = "flipdot/state_font2/state";

  // Text
    const char* T_TEXT_SET   = "flipdot/text/set";
    const char* T_TEXT_STATE = "flipdot/text/state";

    const char* T_TEXT2_SET   = "flipdot/text2/set";
    const char* T_TEXT2_STATE = "flipdot/text2/state";

  // Mode select
    const char* T_SELECT_MODE_SET   = "flipdot/mode/set";
    const char* T_SELECT_MODE_STATE = "flipdot/mode/state";

  // Vars

    bool state_TimeinText = true;
    bool state_showSeconds = true;

    int stateValue = start_mode;

    int state_x = 29;
    int state_y = 0;
    int state_x2 = 0;
    int state_y2 = 0;

    int state_updateRate = 15;
    int state_font = 5;
    int state_font2 = 0;

    String textValue = "Hello World!";
    String text2Value = "Hello World!2";
    bool mqttWasConnected = false;
    unsigned long mqttDisconnectedSince = 0;

    const char* MODE_OPTIONS[] = {
      "Clock BIG",
      "Blinking Text",
      "Static Text",
      "Full ON",
      "Christmas"
    };
    const size_t MODE_OPTION_COUNT = sizeof(MODE_OPTIONS) / sizeof(MODE_OPTIONS[0]);

// ============================================================================


WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

bool isEven(long n){
  return n % 2 == 0;
}
String devId() {
  return "flipdot_" + String((uint32_t)ESP.getEfuseMac(), HEX);
}
void publishDiscovery() {
  const String id = devId();

  // Device description
    const String deviceJson =
      String("\"device\":{") +
        "\"identifiers\":[\"" + id + "\"]," +
        "\"name\":\"Flipdot Controller\"," +
        "\"manufacturer\":\"DIY\"," +
        "\"model\":\"ESP32 + Mobitec\"," +
        "\"sw_version\":\"1.0\"" +
      "}";

  // HELPER:
    // Common availability block 
      const String availabilityJson =
        String("\"availability_topic\":\"") + String(AVAIL_TOPIC) + "\"," +
        "\"payload_available\":\"online\"," +
        "\"payload_not_available\":\"offline\",";

    // Helper: publish retained discovery message
      auto publishConfig = [&](const String& topic, const String& payload) {
        mqtt.publish(topic.c_str(), payload.c_str(), true);
      };

    // Helper: create discovery topic
      auto discoveryTopic = [&](const char* component, const char* objectId) -> String {
        return String(DISCOVERY_PREFIX) + "/" + component + "/" + id + "/" + objectId + "/config";
      };

    // Helper: create switch payload
      auto makeSwitchPayload = [&](const String& name,
                                  const String& uniqueId,
                                  const String& commandTopic,
                                  const String& stateTopic) -> String {
      return String("{") +
        "\"name\":\"" + name + "\"," +
        "\"unique_id\":\"" + uniqueId + "\"," +
        "\"command_topic\":\"" + commandTopic + "\"," +
        "\"state_topic\":\"" + stateTopic + "\"," +
        "\"payload_on\":\"ON\"," +
        "\"payload_off\":\"OFF\"," +
        "\"state_on\":\"ON\"," +
        "\"state_off\":\"OFF\"," +
        availabilityJson +
        deviceJson +
        "}";
      };

    // Helper: create button payload
      auto makeButtonPayload = [&](const String& name,
                                  const String& uniqueId,
                                  const String& commandTopic) -> String {
        return String("{") +
          "\"name\":\"" + name + "\"," +
          "\"unique_id\":\"" + uniqueId + "\"," +
          "\"command_topic\":\"" + commandTopic + "\"," +
          "\"payload_press\":\"PRESS\"," +
          availabilityJson +
          deviceJson +
          "}";
      };

    // Helper: create number payload
      auto makeNumberPayload = [&](const String& name,
                                  const String& uniqueId,
                                  const String& commandTopic,
                                  const String& stateTopic,
                                  int minValue,
                                  int maxValue,
                                  int stepValue) -> String {
        return String("{") +
          "\"name\":\"" + name + "\"," +
          "\"unique_id\":\"" + uniqueId + "\"," +
          "\"command_topic\":\"" + commandTopic + "\"," +
          "\"state_topic\":\"" + stateTopic + "\"," +
          "\"min\":" + String(minValue) + "," +
          "\"max\":" + String(maxValue) + "," +
          "\"step\":" + String(stepValue) + "," +
          "\"mode\":\"box\"," +
          availabilityJson +
          deviceJson +
          "}";
      };

    // Helper: create text payload
      auto makeTextPayload = [&](const String& name,
                                const String& uniqueId,
                                const String& commandTopic,
                                const String& stateTopic,
                                int maxLength) -> String {
        return String("{") +
          "\"name\":\"" + name + "\"," +
          "\"unique_id\":\"" + uniqueId + "\"," +
          "\"command_topic\":\"" + commandTopic + "\"," +
          "\"state_topic\":\"" + stateTopic + "\"," +
          "\"mode\":\"text\"," +
          "\"min\":0," +
          "\"max\":" + String(maxLength) + "," +
          availabilityJson +
          deviceJson +
          "}";
      };

    // Helper: create select payload
    // Helper: convert string array into JSON array
      auto makeJsonStringArray = [&](const char* const* values, size_t count) -> String {
        String json = "[";
        for (size_t i = 0; i < count; ++i) {
          if (i > 0) {
            json += ",";
          }
          json += "\"";
          json += values[i];
          json += "\"";
        }
        json += "]";
        return json;
      };
      const String modeOptionsJson = makeJsonStringArray(MODE_OPTIONS, MODE_OPTION_COUNT);
      auto makeSelectPayload = [&](const String& name,
                                  const String& uniqueId,
                                  const String& commandTopic,
                                  const String& stateTopic,
                                  const String& optionsJson) -> String {
        return String("{") +
          "\"name\":\"" + name + "\"," +
          "\"unique_id\":\"" + uniqueId + "\"," +
          "\"command_topic\":\"" + commandTopic + "\"," +
          "\"state_topic\":\"" + stateTopic + "\"," +
          "\"options\":" + optionsJson + "," +
          availabilityJson +
          deviceJson +
          "}";
      };


  // Topics

    // Switch topics
    const String t_sw_ttt       = discoveryTopic("switch", "sw_ttt");
    const String t_relay1       = discoveryTopic("switch", "relay1");
    const String t_sw_showsec   = discoveryTopic("switch", "sw_showsec");

    // Button topics
    const String t_btn_180      = discoveryTopic("button", "180");
    const String t_btn_apply    = discoveryTopic("button", "apply");

    // Number topics
    const String t_font         = discoveryTopic("number", "state_font");
    const String t_font2        = discoveryTopic("number", "state_font2");
    const String t_rate         = discoveryTopic("number", "state_updateRate");
    const String t_x1           = discoveryTopic("number", "state_x");
    const String t_x2           = discoveryTopic("number", "state_x2");
    const String t_y1           = discoveryTopic("number", "state_y");
    const String t_y2           = discoveryTopic("number", "state_y2");

    // Text topics
    const String t_text1        = discoveryTopic("text", "display");
    const String t_text2        = discoveryTopic("text", "display2");

    // Select topics
    const String t_mode         = discoveryTopic("select", "mode");

  // Payloads
  // Switch payloads
    const String p_sw_ttt = makeSwitchPayload(
      "Time To Text 1",
      id + "_sw_ttt",
      String(T_SW_TTT_SET),
      String(T_SW_TTT_STATE)
    );

    const String p_relay1 = makeSwitchPayload(
      "LIGHTS",
      id + "_relay1",
      String(T_RELAY1_SET),
      String(T_RELAY1_STATE)
    );

    const String p_sw_showsec = makeSwitchPayload(
      "Show Seconds",
      id + "_sw_showsec",
      String(T_SW_SHOWSEC_SET),
      String(T_SW_SHOWSEC_STATE)
    );

  // Button payloads
    const String p_btn_180 = makeButtonPayload(
      "180",
      id + "_180",
      String(T_BTN_180)
    );

    const String p_btn_restart = makeButtonPayload(
      "Restart",
      id + "_restart",
      String(T_BTN_RESTART)
    );

  // Number payloads
    const String p_font = makeNumberPayload(
      "Font",
      id + "_state_font",
      String(T_NUM_FONT_STATE_SET),
      String(T_NUM_FONT_STATE_STATE),
      0, 8, 1
    );

    const String p_font2 = makeNumberPayload(
      "Font Row 2",
      id + "_state_font2",
      String(T_NUM_FONT2_STATE_SET),
      String(T_NUM_FONT2_STATE_STATE),
      0, 8, 1
    );

    const String p_rate = makeNumberPayload(
      "Update_rate",
      id + "_state_updateRate",
      String(T_NUM_RATE_STATE_SET),
      String(T_NUM_RATE_STATE_STATE),
      0, 60, 1
    );

    const String p_x1 = makeNumberPayload(
      "Pos_x",
      id + "_state_x",
      String(T_NUM_X_STATE_SET),
      String(T_NUM_X_STATE_STATE),
      0, 112, 1
    );

    const String p_x2 = makeNumberPayload(
      "Pos_x_2",
      id + "_state_x2",
      String(T_NUM_X2_STATE_SET),
      String(T_NUM_X2_STATE_STATE),
      0, 112, 1
    );

    const String p_y1 = makeNumberPayload(
      "Pos_y",
      id + "_state_y",
      String(T_NUM_Y_STATE_SET),
      String(T_NUM_Y_STATE_STATE),
      -13, 16, 1
    );

    const String p_y2 = makeNumberPayload(
      "Pos_y_2",
      id + "_state_y2",
      String(T_NUM_Y2_STATE_SET),
      String(T_NUM_Y2_STATE_STATE),
      -13, 16, 1
    );

    // Text payloads
    const String p_text1 = makeTextPayload(
      "Row 1 Text",
      id + "_row_1_text",
      String(T_TEXT_SET),
      String(T_TEXT_STATE),
      64
    );

    const String p_text2 = makeTextPayload(
      "Row 2 Text",
      id + "_row_2_text",
      String(T_TEXT2_SET),
      String(T_TEXT2_STATE),
      64
    );

  // Select payloads
    const String p_mode = makeSelectPayload(
      "Modus",
      id + "_mode",
      String(T_SELECT_MODE_SET),
      String(T_SELECT_MODE_STATE),
      modeOptionsJson
    );


  // Publish

    // Switches
    publishConfig(t_sw_ttt,   p_sw_ttt);
    publishConfig(t_relay1,    p_relay1);
    publishConfig(t_sw_showsec,   p_sw_showsec);

    // Buttons
    publishConfig(t_btn_180,   p_btn_180);
    publishConfig(t_btn_apply, p_btn_restart);

    // Numbers
    publishConfig(t_font,      p_font);
    publishConfig(t_font2,     p_font2);
    publishConfig(t_rate,      p_rate);
    publishConfig(t_x1,        p_x1);
    publishConfig(t_x2,        p_x2);
    publishConfig(t_y1,        p_y1);
    publishConfig(t_y2,        p_y2);

    // Text
    publishConfig(t_text1,     p_text1);
    publishConfig(t_text2,     p_text2);

    // Select
    publishConfig(t_mode,      p_mode);
}
void applyRelay(int pin, bool on) {
  on = !on;
  if (RELAY_ACTIVE_LOW) digitalWrite(pin, on ? LOW : HIGH);
  else digitalWrite(pin, on ? HIGH : LOW);
}
void publishStates(bool retain = true) {
  char buf[16];

  snprintf(buf, sizeof(buf), "%d", state_x);
  mqtt.publish(T_NUM_X_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_y);
  mqtt.publish(T_NUM_Y_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_x2);
  mqtt.publish(T_NUM_X2_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_y2);
  mqtt.publish(T_NUM_Y2_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_updateRate);
  mqtt.publish(T_NUM_RATE_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_font);
  mqtt.publish(T_NUM_FONT_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_font2);
  mqtt.publish(T_NUM_FONT2_STATE_STATE, buf, retain);

  mqtt.publish(T_RELAY1_STATE, relay1_On ? "ON" : "OFF", retain);
  mqtt.publish(T_SW_TTT_STATE, state_TimeinText ? "ON" : "OFF", retain);
  mqtt.publish(T_SW_SHOWSEC_STATE, state_showSeconds ? "ON" : "OFF", retain);

  const char* modeText = (stateValue >= 0 && stateValue < (int)MODE_OPTION_COUNT)
    ? MODE_OPTIONS[stateValue]
    : MODE_OPTIONS[0];
  mqtt.publish(T_SELECT_MODE_STATE, modeText, retain);
}
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t(topic);

  String msg = "";
  msg.reserve(length);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  if (t == T_TEXT_SET) {
    textValue = msg;
    if (textValue.length() > 255) textValue = textValue.substring(0, 255);
    textDirty = true;
    mqtt.publish(T_TEXT_STATE, textValue.c_str(), true);
    return;
  }
  if (t == T_TEXT2_SET) {
    text2Value = msg;
    if (text2Value.length() > 255) text2Value = text2Value.substring(0, 255);
    textDirty = true;
    mqtt.publish(T_TEXT2_STATE, text2Value.c_str(), true);
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
  bool msg_state;
  if (msg == "ON" || msg == "1" || msg == "true") msg_state = true;
  else if (msg == "OFF" || msg == "0" || msg == "false") msg_state = false;

  if (t == T_RELAY1_SET) {  //lights
    relay1_On = msg_state;
    applyRelay(RELAY1_PIN, relay1_On);
    mqtt.publish(T_RELAY1_STATE, relay1_On ? "ON" : "OFF", true);
    textDirty = true;
  }
  else if (t == T_SW_TTT_SET) { //TTT
    state_TimeinText = msg_state;
    mqtt.publish(T_SW_TTT_STATE, state_TimeinText ? "ON" : "OFF", true);
    textDirty = true;
  }
  else if (t == T_SW_SHOWSEC_SET) { //Show Seconds
    state_showSeconds = msg_state;
    mqtt.publish(T_SW_SHOWSEC_STATE, state_showSeconds ? "ON" : "OFF", true);
    textDirty = true;
  }
  else if (t == T_BTN_RESTART) { // Button: restart 
    //Serial.printf("Apply pressed\n");
    ESP.restart();
    return;
  }
  else if (t == T_BTN_180) { // Button 180
    //Serial.printf("180 pressed\n");
    last_loop_state = loop_state;
    loop_state = 180;
    stateValue = loop_state;
    textDirty = true;
    return;
  }
  else if (t == T_NUM_X_STATE_SET) {// X
    int v = msg.toInt();           
    if (v < 0) v = 0;
    if (v > 112) v = 112;
    state_x = v;
    publishStates(true);
    textDirty = true;
    return;
  }
  else if (t == T_NUM_Y_STATE_SET) {// Y 
    int v = msg.toInt();           
    if (v < -13) v = -13;
    if (v > 16) v = 16;
    state_y = v;
    publishStates(true);
    textDirty = true;
    return;
  } 
  else if (t == T_NUM_X2_STATE_SET) {// X 2
    int v = msg.toInt();           
    if (v < 0) v = 0;
    if (v > 112) v = 112;
    state_x2 = v;
    publishStates(true);
    textDirty = true;
    return;
  }
  else if (t == T_NUM_Y2_STATE_SET) {// Y 2 
    int v = msg.toInt();           
    if (v < -13) v = -13;
    if (v > 16) v = 16;
    state_y2 = v;
    publishStates(true);
    textDirty = true;
    return;
  }        
  else if (t == T_NUM_RATE_STATE_SET) {// update rate
    int v = msg.toInt();           
    if (v < 0) v = 0;
    if (v > 60) v = 60;
    state_updateRate = v;
    update_rate = v*1000;
    publishStates(true);
    textDirty = true;
    return;
  }  
  else if (t == T_NUM_FONT_STATE_SET) {// font 1
    int v = msg.toInt();           
    if (v < 0) v = 0;
    if (v > 8) v = 0;
    state_font = v;
    font_mode = v;
    publishStates(true);
    textDirty = true;
    return;
  }  
  else if (t == T_NUM_FONT2_STATE_SET) {// font 2
    int v = msg.toInt();           
    if (v < 0) v = 0;
    if (v > 8) v = 0;
    state_font2 = v;
    font2_mode = v;
    publishStates(true);
    textDirty = true;
    return;
  } 
  else if (t == T_SELECT_MODE_SET) {  //new mode select
    if (msg == "Clock BIG") {
      loop_state = 0;
      stateValue = loop_state;
    }
    else if (msg == "Blinking Text") {
      loop_state = 1;
      stateValue = loop_state;
    }
    else if (msg == "Static Text") {
      loop_state = 2;
      stateValue = loop_state;
    }
    else if (msg == "Christmas") {
      loop_state = 3;
      stateValue = loop_state;
    }
    else if (msg == "Full ON") {
      loop_state = 4;
      stateValue = loop_state;
    }
    else{
      loop_state = 0;
      stateValue = loop_state;
    }
    tls_prt_dbg = millis();
    textDirty = true;
    mqtt.publish(T_SELECT_MODE_STATE, msg.c_str(), true);
    return;
  }
}
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  tls_wifi = millis();
  try_wifi = true;

  while (WiFi.status() != WL_CONNECTED  && try_wifi) {
    Serial.printf("Connecting...\n");
    delay(50);
    if(millis() - tls_wifi > 20000){
      try_wifi = false;
    }
  }
}
void ensureMqtt() {
  if (mqtt.connected()) {
    mqttDisconnectedSince = 0;
    return;
  }
  Serial.printf("mqtt Connecting...\n");

  String cid = "flipdot-esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  bool ok = false;

  if (MQTT_USER && MQTT_PASS && MQTT_USER[0] != '\0') {
    ok = mqtt.connect(
      cid.c_str(),
      MQTT_USER, MQTT_PASS,
      AVAIL_TOPIC, 0, true, "offline"
    );
  } 
  else {
    ok = mqtt.connect(
      cid.c_str(),
      AVAIL_TOPIC, 0, true, "offline"
    );
  }

  if (ok) {
    mqtt.setBufferSize(MQTT_BUFFER_SIZE);

    mqtt.subscribe(T_BTN_RESTART);
    mqtt.subscribe(T_BTN_180);
    mqtt.subscribe(T_NUM_X_STATE_SET);
    mqtt.subscribe(T_NUM_Y_STATE_SET);
    mqtt.subscribe(T_NUM_X2_STATE_SET);
    mqtt.subscribe(T_NUM_Y2_STATE_SET);
    mqtt.subscribe(T_NUM_RATE_STATE_SET);
    mqtt.subscribe(T_NUM_FONT_STATE_SET);
    mqtt.subscribe(T_NUM_FONT2_STATE_SET);
    mqtt.subscribe(T_TEXT_SET);
    mqtt.subscribe(T_TEXT2_SET);
    mqtt.subscribe(T_RELAY1_SET);
    mqtt.subscribe(T_SW_TTT_SET);
    mqtt.subscribe(T_SW_SHOWSEC_SET);
    mqtt.subscribe(HA_STATUS_TOPIC);
    mqtt.subscribe(T_SELECT_MODE_SET);

    publishStates(true);

    mqtt.publish(T_TEXT_STATE, textValue.c_str(), true);
    mqtt.publish(T_TEXT2_STATE, text2Value.c_str(), true);
    mqtt.publish(AVAIL_TOPIC, "online", true);
    publishDiscovery();

    mqttWasConnected = true;
    mqttDisconnectedSince = 0;

    Serial.printf("mqtt Connected.\n");
  } 
  else {
    Serial.printf("mqtt connect failed, rc=%d\n", mqtt.state());
    if (!mqttWasConnected) {
      ESP.restart();
      return;
    }
    if (mqttDisconnectedSince == 0) {
      mqttDisconnectedSince = millis();
    }

    if (millis() - mqttDisconnectedSince >= 300000UL) {
      ESP.restart();
      return;
    }
  }
}
void check_connection(){
  if(try_wifi){
    if (WiFi.status() == WL_CONNECTED) {
      if (mqtt.connected()) {
        mqtt.loop();
        handleTimeSync();
      } 
      else {     
        if (millis() - lastTry > 2000) {
          lastTry = millis();
          ensureMqtt();
        }
      }
    }
  }
  else{
    handle_controller();
  }  
}  





