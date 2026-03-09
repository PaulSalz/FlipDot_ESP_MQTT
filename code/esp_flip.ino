#include "0_arduino_secrets.h"
#include <Arduino.h>
#include <vector>
#include <time.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <algorithm> 
#include <Bluepad32.h>  

int start_mode = 0;
int loop_state = start_mode;
int last_loop_state = start_mode;
int font_mode = 5;
int font2_mode = 0;
bool try_wifi = true;
long tls_wifi = 0;
long tls_prt_dbg = 0;
long tls_font = 0;
long tls_case[7] = {0};
long tls_interval = 500, update_rate = 0;
bool b_tgl = false;

#include "c_display.h"
#include "a_wireless_mqtt.h"
#include "b_controller.h" 
#include "d_time.h" 


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
      single_msg("Wifi done");
      delay(500);
      single_msg("MQTT . . .");
      mqtt.setServer(MQTT_HOST, MQTT_PORT);
      mqtt.setBufferSize(mqtt_buf);
      mqtt.setCallback(mqttCallback);
      ensureMqtt();
      single_msg("CONNECTED");
      initTimeNTP();
      single_msg("NTP Time ok");
      delay(1000);
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
    delay(1000);
    Serial.printf("Setup done\n");
  }


//--------------------------------------------------------------------------------------------------------------------------------

  void loop() {
    if(try_wifi){
      ensureMqtt();
      mqtt.loop();
      handleTimeSync();
    }
    else{
      handle_controller();
    }


    switch (loop_state) {
      case 0:// Clock
        if(millis()-tls_case[loop_state] > 50 + update_rate){
          tls_case[loop_state] = millis();
          switch_font(font_mode);
          showClockOnFlipdot();
          flipdot.show();
        }
        break;  
      case 1:// text blinkend
        if(millis()-tls_case[loop_state] > tls_interval + update_rate){
          tls_case[loop_state] = millis();       
          if(b_tgl) {
            flipdot.setPosition(state_x, state_y);
            switch_font(font_mode);
            if(state_ttt){
              showClockOnFlipdot();
            }
            else{
              flipdot.printText(textValue);
            }
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
            switch_font(font_mode);
            if(state_ttt){
              textDirty = true;
              showClockOnFlipdot();
            }
            else{
              textDirty = false;
              flipdot.clearBuffer();
              flipdot.setPosition(state_x, state_y);
              flipdot.printText(textValue);
            }  
            flipdot.show();
          }
        }
        break;  
      case 3:// text double
        if(millis()-tls_case[loop_state] > 50 + update_rate){
          if(font_mode > 1){
            font_mode = 0;
            state_font = 0;
            switch_font(font_mode);
            publishState_font(true);
          }          
          tls_case[loop_state] = millis();          
          if(state_ttt){
            //combined
            showClockOnFlipdot();
          }
          else{
            //row1
            flipdot.clearBuffer();
            switch_font(font_mode);
            flipdot.setPosition(state_x, state_y);
            flipdot.printText(textValue);
            //row2
            switch_font(font2_mode);
            flipdot.setPosition(state_x2, state_y2);
            flipdot.printText(text2Value);
          }   
          flipdot.show();
        }
        break;
      case 4:// Xmas und Bäume
        if(millis()-tls_case[loop_state] > tls_interval + update_rate){
          tls_case[loop_state] = millis();  
          switch_font(4);    
          if(b_tgl) {
            flipdot.clearBuffer();
            bm_tree.clear();
            for(int i=3; i<103; i+=12){
              drawTree3LevelsInto(bm_tree, i, 0);
            }
            flipdot.setPosition(0, 0);
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

      case 5:// Full on
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
        if(millis()-tls_prt_dbg > 2000){
          Serial.printf("ENDE!  loop_state: %d\n", loop_state);
          tls_prt_dbg = millis();
          single_msg("mode!");
        }
        break;
    }
  }



//--------------------------------------------------------------------------------------------------------------------------------





























