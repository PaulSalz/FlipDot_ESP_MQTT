#include <cstdint>
#pragma once

int lastShownSec = -1;
int lastShownMin = -1;
static const char* TZ_BERLIN = "CET-1CEST,M3.5.0/2,M10.5.0/3";
bool   timeIsValid       = false;
time_t nextNtpSyncEpoch  = 0;
uint32_t nextRetryMs     = 0;
// ---------- Num width ----------

  const uint8_t DIGIT_W_FONT0[10] = {
    6, // 0
    2, // 1
    5, // 2
    5, // 3
    6, // 4
    5, // 5
    5, // 6
    5, // 7
    5, // 8
    5  // 9
  };

  const uint8_t DIGIT_W_FONT1[10] = {
    7, // 0
    4, // 1
    6, // 2
    6, // 3
    6, // 4
    6, // 5
    6, // 6
    5, // 7
    6, // 8
    6  // 9
  };
  const uint8_t COLON_W_FONT0 = 2;
  const uint8_t COLON_W_FONT1 = 2;
  const int CLOCK_RIGHT_EDGE = 111;

  uint16_t getTimeWidth(const char* s, uint8_t fontMode) {
    uint16_t w = 0;
    uint8_t max_digits = 4;

    for (size_t i = 0; s[i] != '\0'; i++) {
      char c = s[i];
      if (c >= '0' && c <= '9') {
        if(max_digits>0){
          uint8_t d = c - '0';
          if (fontMode == 0) {
            w += DIGIT_W_FONT0[d];
            w++;
          } 
          else {
            w += DIGIT_W_FONT1[d];
            w++;
          }
          max_digits--;
        }
        else{
          w += 7;
        }
      }
      else if (c == ':') {
        if (fontMode == 0) {
          w += COLON_W_FONT0;
        } 
        else {
          w += COLON_W_FONT1;
        }
      }
      else{
        w++;
      }
    }
    return w;
  }

  int getRightAlignedClockX(const char* s, uint8_t fontMode, int rightEdge) {
    uint16_t w = getTimeWidth(s, fontMode);
    return rightEdge - (int)w + 2;
  } 
  

//setup stuff
  void scheduleNextMidnightSync(const tm &nowTm) {
    tm t = nowTm;
    t.tm_mday += 1;     // nächster Tag
    t.tm_hour = 0;
    t.tm_min  = 0;
    t.tm_sec  = 5;      // 5s nach Mitternacht
    nextNtpSyncEpoch = mktime(&t);   // mktime arbeitet in lokaler TZ (inkl. DST)
  }
  bool syncTimeFromNTP() {
    // WICHTIG: nicht configTime(0,0,...) -> das ist UTC
    configTzTime(TZ_BERLIN, "pool.ntp.org", "time.cloudflare.com");

    struct tm tmNow;
    for (int i = 0; i < 30; i++) {
      if (getLocalTime(&tmNow, 500)) {
        timeIsValid = true;
        scheduleNextMidnightSync(tmNow);
        return true;
      }
      delay(200);
    }

    Serial.println("NTP sync failed");
    timeIsValid = false;
    return false;
  }

  void initTimeNTP() {
    setenv("TZ", TZ_BERLIN, 1);
    tzset();

    syncTimeFromNTP();
  }
  void handleTimeSync() {
    const uint32_t nowMs = millis();
    if (!timeIsValid) {
      if (nowMs >= nextRetryMs) {
        if (!syncTimeFromNTP()) {
          nextRetryMs = nowMs + 60UL * 1000UL; // Retry in 60s
        }
      }
      return;
    }

    const time_t nowEpoch = time(nullptr);
    if (nowEpoch >= nextNtpSyncEpoch) {
      if (!syncTimeFromNTP()) {
        nextNtpSyncEpoch = nowEpoch + 10 * 60;
      }
    }
  }
//

void ClockwithText(uint8_t update_rate) {
  if (!timeIsValid) return;
  time_t nowEpoch = time(nullptr);
  if (nowEpoch <= 0) return;

  struct tm tmNow;
  localtime_r(&nowEpoch, &tmNow);

  static bool lastShowSecondsState = false;
  static bool firstCall = true;
  bool forceRedraw = firstCall || (state_showSeconds != lastShowSecondsState) || textDirty;
  textDirty = false;
  firstCall = false;
  lastShowSecondsState = state_showSeconds;

  if (forceRedraw) {
    lastShownSec = -1;
    lastShownMin = -1;
  }

  struct tm tmDisplay = tmNow;

  if (state_showSeconds && update_rate > 1 && update_rate < 60) {
    tmDisplay.tm_sec = (tmNow.tm_sec / update_rate) * update_rate;
  }

  if (!forceRedraw) {
    if (state_showSeconds) {
      if (tmNow.tm_min == lastShownMin && tmDisplay.tm_sec == lastShownSec) {
        return;
      }
    } else {
      if (tmNow.tm_min == lastShownMin) {
        return;
      }
    }
  }

  lastShownMin = tmNow.tm_min;
  lastShownSec = state_showSeconds ? tmDisplay.tm_sec : -1;

  char buf_time[16];
  if (state_showSeconds) {
    strftime(buf_time, sizeof(buf_time), "%H:%M:%S", &tmDisplay);
  } else {
    strftime(buf_time, sizeof(buf_time), "%H:%M", &tmDisplay);
  }

  flipdot.clearBuffer();

  if (loop_state == 0) {  // big clock
    switch_font(5);
    if (state_showSeconds) {
      flipdot.setPosition(26, -5);
    } else {
      flipdot.setPosition(36, -5);
    }
    flipdot.printText(buf_time);
  }
  else { // small clock with text
    // row 1
    switch_font(font_mode);
    flipdot.setPosition(state_x, state_y);
    flipdot.printText(textValue);

    // clock on the right
    uint8_t clockFont = state_showSeconds ? 0 : 1;
    
    int clockX = getRightAlignedClockX(buf_time, clockFont, CLOCK_RIGHT_EDGE);

    switch_font(clockFont);
    flipdot.setPosition(clockX, -5);
    flipdot.printText(buf_time);

    // row 2
    switch_font(font2_mode);
    flipdot.setPosition(state_x2, state_y2);
    flipdot.printText(text2Value);
  }
}

















