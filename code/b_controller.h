  #pragma once

  
  GamepadPtr pads[BP32_MAX_GAMEPADS];

   void onConnectedGamepad(GamepadPtr gp) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
      if (!pads[i]) { pads[i] = gp; break; }
    }
    Serial.println("PS4 connected");
  }

   void onDisconnectedGamepad(GamepadPtr gp) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
      if (pads[i] == gp) { pads[i] = nullptr; break; }
    }
    Serial.println("PS4 disconnected");
  }
  bool prevA=false, prevB=false, prevX=false, prevY=false;

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
          publishStates(true);
        }
        Serial.printf("PS4: mode=%d\n", stateValue);
      }
      prevA = A; prevB = B; prevX = X; prevY = Y;
    }
  }
