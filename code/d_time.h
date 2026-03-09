#pragma once

int lastShownSec = -1;
static const char* TZ_BERLIN = "CET-1CEST,M3.5.0/2,M10.5.0/3";
bool   timeIsValid       = false;
time_t nextNtpSyncEpoch  = 0;
uint32_t nextRetryMs     = 0;


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


void showClockOnFlipdot() {
  if (!timeIsValid) return;

  time_t nowEpoch = time(nullptr);
  if (nowEpoch <= 0) return;

  struct tm tmNow;
  localtime_r(&nowEpoch, &tmNow);

  if (tmNow.tm_sec == lastShownSec) return;
  lastShownSec = tmNow.tm_sec;

  char buf[16];
  strftime(buf, sizeof(buf), "%H:%M:%S", &tmNow);

  flipdot.clearBuffer();
  flipdot.setPosition(state_x, state_y);
  switch_font(font_mode);
  flipdot.printText(buf);
  if(loop_state == 3){
    switch_font(font2_mode);
    flipdot.setPosition(state_x2, state_y2);
    flipdot.printText(text2Value);
  }
}



