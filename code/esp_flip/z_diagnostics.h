#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <esp_system.h>

enum RebootCause : uint8_t {
  REBOOT_UNKNOWN,
  REBOOT_MQTT_FIRST_CONNECT_FAIL,
  REBOOT_MQTT_TIMEOUT_5MIN,
  REBOOT_MANUAL_CMD
};

struct BootInfo {
  uint32_t bootCount;
  uint8_t lastResetReason;
  uint8_t plannedRebootCause;
  int lastMqttState;
  uint32_t lastUptimeMs;
};

inline BootInfo bootInfo;

inline const char* resetReasonToText(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:   return "power_on";
    case ESP_RST_EXT:       return "external_pin";
    case ESP_RST_SW:        return "software";
    case ESP_RST_PANIC:     return "panic";
    case ESP_RST_INT_WDT:   return "int_wdt";
    case ESP_RST_TASK_WDT:  return "task_wdt";
    case ESP_RST_WDT:       return "wdt";
    case ESP_RST_DEEPSLEEP: return "deep_sleep";
    case ESP_RST_BROWNOUT:  return "brownout";
    case ESP_RST_SDIO:      return "sdio";
    default:                return "unknown";
  }
}

inline const char* rebootCauseToText(uint8_t c) {
  switch (c) {
    case REBOOT_MQTT_FIRST_CONNECT_FAIL: return "mqtt_first_connect_fail";
    case REBOOT_MQTT_TIMEOUT_5MIN:       return "mqtt_timeout_5min";
    case REBOOT_MANUAL_CMD:              return "manual_cmd";
    default:                             return "unknown";
  }
}

inline void diagBegin() {
  Preferences p;
  p.begin("flipdiag", false);

  bootInfo.bootCount = p.getUInt("boot_cnt", 0) + 1;
  p.putUInt("boot_cnt", bootInfo.bootCount);

  bootInfo.lastResetReason = (uint8_t)esp_reset_reason();
  bootInfo.plannedRebootCause = p.getUChar("rb_cause", REBOOT_UNKNOWN);
  bootInfo.lastMqttState = p.getInt("mqtt_state", 0);
  bootInfo.lastUptimeMs = p.getUInt("uptime_ms", 0);

  p.putUChar("rb_cause", REBOOT_UNKNOWN);
  p.putInt("mqtt_state", 0);
  p.putUInt("uptime_ms", 0);

  p.end();
}

inline void markPlannedRestart(uint8_t cause, int mqttState) {
  Preferences p;
  p.begin("flipdiag", false);
  p.putUChar("rb_cause", cause);
  p.putInt("mqtt_state", mqttState);
  p.putUInt("uptime_ms", millis());
  p.end();
}

inline String bootSummaryJson() {
  String s = "{";
  s += "\"boot_count\":" + String(bootInfo.bootCount) + ",";
  s += "\"reset_reason\":\"" + String(resetReasonToText((esp_reset_reason_t)bootInfo.lastResetReason)) + "\",";
  s += "\"planned_reboot\":\"" + String(rebootCauseToText(bootInfo.plannedRebootCause)) + "\",";
  s += "\"last_mqtt_state\":" + String(bootInfo.lastMqttState) + ",";
  s += "\"last_uptime_ms\":" + String(bootInfo.lastUptimeMs);
  s += "}";
  return s;
}