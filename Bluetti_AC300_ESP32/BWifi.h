#ifndef BWIFI_H
#define BWIFI_H
#include "Arduino.h"
#include "config.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

typedef struct{
  int  salt = EEPROM_SALT;
  char mqtt_server[40] = MQTT_SERVER;
  char mqtt_port[6]  = "1883";
  char mqtt_username[40] = "";
  char mqtt_password[40] = "";
  char bluetti_device_id[40] = BLUETTI_DEV_ID;
} ESPBluettiSettings;

extern ESPBluettiSettings get_esp32_bluetti_settings();
extern void initBWifi(bool resetWifi);
//extern void handleWebserver();
//void handleNotFound();
//void handleRoot();

#endif
