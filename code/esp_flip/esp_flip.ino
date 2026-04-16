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
long lastTry = 0;
long tls_case[7] = {0};
long tls_interval = 500, update_rate = 0;
bool b_tgl = false;
bool textDirty = true; 
bool relay1_On = true;
static unsigned long lastDiagUpdate = 0;

#include "z_diagnostics.h"
#include "c_display.h"
#include "a_wireless_mqtt.h"
#include "b_controller.h" 
#include "d_time.h" 



//--------------------------------------------------------------------------------------------------------------------------------
  
  void setup() {
    Serial.begin(115200);
    delay(2500);

    diagBegin();
    Serial.printf("\n\nHello World\n");
    Serial.printf("Boot count: %u\n", bootInfo.bootCount);
    Serial.printf("Reset reason: %s\n",
      resetReasonToText((esp_reset_reason_t)bootInfo.lastResetReason));
    Serial.printf("Planned reboot cause: %s\n",
      rebootCauseToText(bootInfo.plannedRebootCause));

    drawBMfull(bmFull);
    flipdot.begin(4800, UART_RX_PIN, UART_TX_PIN);
    Serial.printf("\n\nHello World\n");
    pinMode(RELAY1_PIN, OUTPUT);
    applyRelay(RELAY1_PIN, relay1_On);

    single_msg("connecting . . .");
    Serial.printf("Starting WIFI\n");
    if(try_wifi){
      connectWiFi();
      //single_msg("Wifi done");
      delay(500);
      //single_msg("MQTT . . .");
      mqtt.setServer(MQTT_HOST, MQTT_PORT);
      mqtt.setBufferSize(MQTT_BUFFER_SIZE);
      mqtt.setCallback(mqttCallback);
      ensureMqtt();
      single_msg("CONNECTED");
      initTimeNTP();
      //single_msg("NTP Time ok");
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
    check_connection();    


    switch (loop_state) {
      case 0:// Clock big
        if(millis()-tls_case[loop_state] > 50 + update_rate){
          tls_case[loop_state] = millis();
          ClockwithText(update_rate);
          flipdot.show();
        }
        break;  
      case 1:// text blinkend
        if(millis()-tls_case[loop_state] > tls_interval + update_rate){
          tls_case[loop_state] = millis();       
          if(b_tgl) {
            if(state_TimeinText){
              //combined
              ClockwithText(update_rate);
              flipdot.show();
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
              flipdot.show();
            }   
          }
          else{
            flipdot.clearBuffer();
            flipdot.show();
          }
          b_tgl = !b_tgl;
        }
        break;
      case 2:// text static
        if(millis()-tls_case[loop_state] > 50 + update_rate){       
          tls_case[loop_state] = millis();  
          if(state_TimeinText){
            //combined
            ClockwithText(update_rate);
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
      case 3:// Xmas und Bäume
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

      case 4:// Full on
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

    if (mqtt.connected() && millis() - lastDiagUpdate > 60000UL) {
      lastDiagUpdate = millis();

      char buf[16];
      snprintf(buf, sizeof(buf), "%lu", millis() / 1000UL);
      mqtt.publish(T_DIAG_UPTIME, buf, true);
    }

  }


//--------------------------------------------------------------------------------------------------------------------------------





























