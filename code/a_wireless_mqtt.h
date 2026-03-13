#pragma once


class Bitmap;  

// ===== WLAN =====
  const char* WIFI_SSID = SECRET_SSID;
  const char* WIFI_PASS = SECRET_PASS;

// ===== MQTT (Broker = Home Assistant / Mosquitto Add-on IP) =====
  const char* MQTT_HOST = SECRET_HOST;
  const uint16_t MQTT_PORT = 1883;
  const char* MQTT_USER = SECRET_MUSER;       
  const char* MQTT_PASS = SECRET_MPASS; 
  const int mqtt_buf = 8192;

// ===== Relais Pins =====
  const int RELAY1_PIN = 19;
  const bool RELAY_ACTIVE_LOW = true;
  bool relay1_On = true;

// ===== Topics =====
  const char* T_RELAY1_SET = "flipdot/relay1/set";
  const char* T_RELAY1_STATE = "flipdot/relay1/state";

  const char* T_BTN_TTT_SET = "flipdot/btn_ttt/set";
  const char* T_BTN_TTT_STATE = "flipdot/btn_ttt/state";
  bool state_ttt = true;

  const char* T_BTN_APPLY_CMD   = "flipdot/apply/press";
  const char* T_BTN_180   = "flipdot/180/press";

  const char* T_NUM_STATE_SET   = "flipdot/state/set";
  const char* T_NUM_STATE_STATE = "flipdot/state/state";
  int stateValue = start_mode;  // 0..10

  const char* T_NUM_X_STATE_SET   = "flipdot/state_x/set";
  const char* T_NUM_X_STATE_STATE = "flipdot/state_x/state";
  int state_x = 29;  

  const char* T_NUM_Y_STATE_SET   = "flipdot/state_y/set";
  const char* T_NUM_Y_STATE_STATE = "flipdot/state_Y/state";
  int state_y = 0;  

  const char* T_NUM_X2_STATE_SET   = "flipdot/state_x2/set";
  const char* T_NUM_X2_STATE_STATE = "flipdot/state_x2/state";
  int state_x2 = 0;  

  const char* T_NUM_Y2_STATE_SET   = "flipdot/state_y2/set";
  const char* T_NUM_Y2_STATE_STATE = "flipdot/state_Y2/state";
  int state_y2 = 0; 

  const char* T_NUM_RATE_STATE_SET   = "flipdot/state_rate/set";
  const char* T_NUM_RATE_STATE_STATE = "flipdot/state_rate/state";
  int state_rate = 0; 

  const char* T_NUM_FONT_STATE_SET   = "flipdot/state_font/set";
  const char* T_NUM_FONT_STATE_STATE = "flipdot/state_font/state";
  int state_font = 5; 

  const char* T_NUM_FONT2_STATE_SET   = "flipdot/state_font2/set";
  const char* T_NUM_FONT2_STATE_STATE = "flipdot/state_font2/state";
  int state_font2 = 0; 

  const char* T_TEXT_SET   = "flipdot/text/set";
  const char* T_TEXT_STATE = "flipdot/text/state";
  String textValue = "Hello World!";

  const char* T_TEXT2_SET   = "flipdot/text2/set";
  const char* T_TEXT2_STATE = "flipdot/text2/state";
  String text2Value = "Hello World!2";
  bool textDirty = true;  

  const char* T_SELECT_MODE_SET   = "rollo/mode/set";
  const char* T_SELECT_MODE_STATE = "rollo/mode/state";

  const char* AVAIL_TOPIC = "flipdot/availability";
  const char* HA_STATUS_TOPIC = "homeassistant/status";
  const char* DISCOVERY_PREFIX = "homeassistant";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

bool isEven(long n){
  return n % 2 == 0;
}
String devId() {
  return "flipdot_" + String((uint32_t)ESP.getEfuseMac(), HEX);
}
void publishDiscovery() {
  String id = devId();
  String devObj =
    String("\"device\":{") +
      "\"identifiers\":[\"" + id +
      "\"]," + "\"name\":\"Flipdot Controller\"," +
      "\"manufacturer\":\"DIY\"," + "\"model\":\"ESP32 + Mobitec\"," +
      "\"sw_version\":\"1.0\"" + "}";

  String t1 = String(DISCOVERY_PREFIX) + "/switch/" + id + "/relay1/config";
  String p1 =
    String("{") +
      "\"name\":\"LIGHTS\"," +
      "\"unique_id\":\"" + id +
      "_relay1\"," + "\"command_topic\":\"" +
      String(T_RELAY1_SET) + "\"," + "\"state_topic\":\"" +
      String(T_RELAY1_STATE) + "\"," + "\"payload_on\":\"ON\",\"payload_off\":\"OFF\"," +
      "\"state_on\":\"ON\",\"state_off\":\"OFF\"," + "\"availability_topic\":\"" +
      String(AVAIL_TOPIC) + "\"," + "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
      devObj + "}";
  mqtt.publish(t1.c_str(), p1.c_str(), true);

  String t_ttt = String(DISCOVERY_PREFIX) + "/switch/" + id + "/btn_ttt/config";
  String p_ttt =
    String("{") +
      "\"name\":\"Time To Text 1\"," +
      "\"unique_id\":\"" + id + "_btn_ttt\"," + 
      "\"command_topic\":\"" + String(T_BTN_TTT_SET) + "\"," +
      "\"state_topic\":\"" + String(T_BTN_TTT_STATE) +
      "\"," + "\"payload_on\":\"ON\",\"payload_off\":\"OFF\"," +
      "\"state_on\":\"ON\",\"state_off\":\"OFF\"," + "\"availability_topic\":\"" +
      String(AVAIL_TOPIC) + "\"," + "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
      devObj + "}";
  mqtt.publish(t_ttt.c_str(), p_ttt.c_str(), true);

  String tb = String(DISCOVERY_PREFIX) + "/button/" + id + "/apply/config";
  String tb_180 = String(DISCOVERY_PREFIX) + "/button/" + id + "/180/config";

  String tn = String(DISCOVERY_PREFIX) + "/number/" + id + "/state/config";
  String tx = String(DISCOVERY_PREFIX) + "/number/" + id + "/state_x/config";
  String ty = String(DISCOVERY_PREFIX) + "/number/" + id + "/state_y/config";
  String trate = String(DISCOVERY_PREFIX) + "/number/" + id + "/state_rate/config";
  String tfont = String(DISCOVERY_PREFIX) + "/number/" + id + "/state_font/config";
  String tfont2 = String(DISCOVERY_PREFIX) + "/number/" + id + "/state_font2/config";

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
    "\"min\":-13,\"max\":16,\"step\":1," +
    "\"mode\":\"box\"," +
    "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
    "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
    devObj +
    "}";  
  String prate =
    String("{") +
    "\"name\":\"Update_rate\"," +
    "\"unique_id\":\"" + id + "_state_rate\"," +
    "\"command_topic\":\"" + String(T_NUM_RATE_STATE_SET) + "\"," +
    "\"state_topic\":\"" + String(T_NUM_RATE_STATE_STATE) + "\"," +
    "\"min\":0,\"max\":60000,\"step\":250," +
    "\"mode\":\"box\"," +
    "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
    "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
    devObj +
    "}";  
  String pfont =
    String("{") +
    "\"name\":\"Font\"," +
    "\"unique_id\":\"" + id + "_state_font\"," +
    "\"command_topic\":\"" + String(T_NUM_FONT_STATE_SET) + "\"," +
    "\"state_topic\":\"" + String(T_NUM_FONT_STATE_STATE) + "\"," +
    "\"min\":0,\"max\":8,\"step\":1," +
    "\"mode\":\"box\"," +
    "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
    "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
    devObj +
    "}";  
  String pfont2 =
    String("{") +
    "\"name\":\"Font Row 2\"," +
    "\"unique_id\":\"" + id + "_state_font2\"," +
    "\"command_topic\":\"" + String(T_NUM_FONT2_STATE_SET) + "\"," +
    "\"state_topic\":\"" + String(T_NUM_FONT2_STATE_STATE) + "\"," +
    "\"min\":0,\"max\":8,\"step\":1," +
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
  mqtt.publish(trate.c_str(), prate.c_str(), true);
  mqtt.publish(tfont.c_str(), pfont.c_str(), true);
  mqtt.publish(tfont2.c_str(), pfont2.c_str(), true);

  String tt = String(DISCOVERY_PREFIX) + "/text/" + id + "/display/config";
  String pt =
    String("{") +
    "\"name\":\"Row 1 Text\"," +
    "\"unique_id\":\"" + id + "_row_1_text\"," +
    "\"command_topic\":\"" + String(T_TEXT_SET) + "\"," +
    "\"state_topic\":\"" + String(T_TEXT_STATE) + "\"," +
    "\"mode\":\"text\"," +
    "\"min\":0,\"max\":64," +   // 64 z.B. sinnvoll fürs Flipdot; max in HA ist 255
    "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
    "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
    devObj +
    "}";
  mqtt.publish(tt.c_str(), pt.c_str(), true);

  String tt2 = String(DISCOVERY_PREFIX) + "/text/" + id + "/display2/config";
  String pt2 =
    String("{") +
    "\"name\":\"Row 2 Text\"," +
    "\"unique_id\":\"" + id + "_row_2_text\"," +
    "\"command_topic\":\"" + String(T_TEXT2_SET) + "\"," +
    "\"state_topic\":\"" + String(T_TEXT2_STATE) + "\"," +
    "\"mode\":\"text\"," +
    "\"min\":0,\"max\":64," +   // 64 z.B. sinnvoll fürs Flipdot; max in HA ist 255
    "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
    "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
    devObj +
    "}";
  mqtt.publish(tt2.c_str(), pt2.c_str(), true);

  String tx2 = String(DISCOVERY_PREFIX) + "/number/" + id + "/state_x2/config";
  String ty2 = String(DISCOVERY_PREFIX) + "/number/" + id + "/state_y2/config";
  String px2 =
    String("{") +
    "\"name\":\"Pos_x_2\"," +
    "\"unique_id\":\"" + id + "_state_x2\"," +
    "\"command_topic\":\"" + String(T_NUM_X2_STATE_SET) + "\"," +
    "\"state_topic\":\"" + String(T_NUM_X2_STATE_STATE) + "\"," +
    "\"min\":0,\"max\":112,\"step\":1," +
    "\"mode\":\"box\"," +
    "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
    "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
    devObj +
    "}";
  String py2 =
    String("{") +
    "\"name\":\"Pos_y_2\"," +
    "\"unique_id\":\"" + id + "_state_y2\"," +
    "\"command_topic\":\"" + String(T_NUM_Y2_STATE_SET) + "\"," +
    "\"state_topic\":\"" + String(T_NUM_Y2_STATE_STATE) + "\"," +
    "\"min\":-13,\"max\":16,\"step\":1," +
    "\"mode\":\"box\"," +
    "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
    "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
    devObj +
    "}";  
  mqtt.publish(tx2.c_str(), px2.c_str(), true);
  mqtt.publish(ty2.c_str(), py2.c_str(), true);

  String t_mode_select = String(DISCOVERY_PREFIX) + "/select/" + id + "/mode/config";
  String p_mode_select =
    String("{") +
    "\"name\":\"Modus\"," +
    "\"unique_id\":\"" + id + "_mode\"," +
    "\"command_topic\":\"" + String(T_SELECT_MODE_SET) + "\"," +
    "\"state_topic\":\"" + String(T_SELECT_MODE_STATE) + "\"," +
    "\"options\":[\"Static Text\",\"Blinking Text\",\"Wipe\",\"Christmas\"]," +
    "\"availability_topic\":\"" + String(AVAIL_TOPIC) + "\"," +
    "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"," +
    devObj +
    "}";

  mqtt.publish(t_mode_select.c_str(), p_mode_select.c_str(), true);

}
void applyRelay(int pin, bool on) {
  on = !on;
  if (RELAY_ACTIVE_LOW) digitalWrite(pin, on ? LOW : HIGH);
  else digitalWrite(pin, on ? HIGH : LOW);
}
void publishStates(bool retain=true) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", stateValue);
  mqtt.publish(T_NUM_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_x);
  mqtt.publish(T_NUM_X_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_y);
  mqtt.publish(T_NUM_Y_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_x2);
  mqtt.publish(T_NUM_X2_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_y2);
  mqtt.publish(T_NUM_Y2_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_rate);
  mqtt.publish(T_NUM_RATE_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_font);
  mqtt.publish(T_NUM_FONT_STATE_STATE, buf, retain);

  snprintf(buf, sizeof(buf), "%d", state_font2);
  mqtt.publish(T_NUM_FONT2_STATE_STATE, buf, retain);

  mqtt.publish(T_RELAY1_STATE, relay1_On ? "ON" : "OFF", retain);
  mqtt.publish(T_BTN_TTT_STATE, state_ttt ? "ON" : "OFF", retain);
}
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t(topic);

  String msg;
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
  if (t == T_BTN_TTT_SET) { //TTT
    state_ttt = msg_state;
    mqtt.publish(T_BTN_TTT_STATE, state_ttt ? "ON" : "OFF", true);
    textDirty = true;
  }
  else if (t == T_BTN_APPLY_CMD) { // Button: restart 
    //Serial.printf("Apply pressed\n");
    ESP.restart();
    return;
  }
  else if (t == T_BTN_180) { // Button 180
    //Serial.printf("180 pressed\n");
    last_loop_state = loop_state;
    loop_state = 180;
    textDirty = true;
    return;
  }
  else if (t == T_NUM_STATE_SET) {// Number: loop State 
    int v = msg.toInt();           
    if (v < 0) v = 0;
    if (v > 10) v = 10;
    stateValue = v;
    loop_state = stateValue;
    publishStates(true);
    tls_prt_dbg = millis();
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
    if (v < -13) v = 0;
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
    if (v < -13) v = 0;
    if (v > 16) v = 16;
    state_y2 = v;
    publishStates(true);
    textDirty = true;
    return;
  }        
  else if (t == T_NUM_RATE_STATE_SET) {// update rate
    int v = msg.toInt();           
    if (v < 0) v = 0;
    if (v > 60000) v = 0;
    state_rate = v;
    update_rate = v;
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
    if (msg == "Static Text") {
      loop_state = 1;
    }
    else if (msg == "Blinking Text") {
      loop_state = 2;
    }
    else if (msg == "Wipe") {
      loop_state = 3;
    }
    else if (msg == "Christmas") {
      loop_state = 4;
    }
    else{
      loop_state = 0;
    }
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
      mqtt.subscribe(T_NUM_X2_STATE_SET);
      mqtt.subscribe(T_NUM_Y2_STATE_SET);
      mqtt.subscribe(T_NUM_RATE_STATE_SET);
      mqtt.subscribe(T_NUM_FONT_STATE_SET);
      mqtt.subscribe(T_NUM_FONT2_STATE_SET);
      mqtt.subscribe(T_TEXT_SET);
      mqtt.subscribe(T_TEXT2_SET);
      mqtt.subscribe(T_RELAY1_SET);
      mqtt.subscribe(T_BTN_TTT_SET);
      mqtt.subscribe(HA_STATUS_TOPIC);

      publishStates(true);

      mqtt.publish(T_TEXT_STATE, textValue.c_str(), true);
      mqtt.publish(T_TEXT2_STATE, text2Value.c_str(), true);
      mqtt.publish(AVAIL_TOPIC, "online", true);
      publishDiscovery();
      
      Serial.printf("mqtt Connected.\n");
    } 
    else {
      Serial.printf("mqtt connect failed, rc=%d\n", mqtt.state());
      delay(2500);
      ESP.restart();
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





