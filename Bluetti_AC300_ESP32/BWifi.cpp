#include "BluettiConfig.h"
#include "BWifi.h"
#include "BTooth.h"
#include "MQTT.h"
/*
#include <EEPROM.h>
#include <WiFiManager.h>
#include <WebServer.h>

WebServer server(80);
*/
bool shouldSaveConfig = false;

char mqtt_server[40] = "192.168.1.20";
char mqtt_port[6]  = "1883";
char bluetti_device_id[40] = "AC3002213000707569";


/************************************************************/
void saveConfigCallback () {
  shouldSaveConfig = true;
}

ESPBluettiSettings wifiConfig;

/************************************************************/
ESPBluettiSettings get_esp32_bluetti_settings(){
    return wifiConfig;  
}
/*
void eeprom_read(){
  Serial.println("Loading Values from EEPROM");
  EEPROM.begin(512);
  EEPROM.get(0, wifiConfig);
  EEPROM.end();
}

void eeprom_saveconfig(){
  Serial.println("Saving Values to EEPROM");
  EEPROM.begin(512);
  EEPROM.put(0, wifiConfig);
  EEPROM.commit();
  EEPROM.end();
}
*/

/************************************************************/
void initBWifi(bool resetWifi){
  /*
  eeprom_read();
  
  if (wifiConfig.salt != EEPROM_SALT) {
    Serial.println("Invalid settings in EEPROM, trying with defaults");
    ESPBluettiSettings defaults;
    wifiConfig = defaults;
  }
  

  WiFiManagerParameter custom_mqtt_server("server", "MQTT Server Address", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT Server Port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_username("username", "MQTT Username", "", 40);
  WiFiManagerParameter custom_mqtt_password("password", "MQTT Password", "", 40, "type=password");
  WiFiManagerParameter custom_bluetti_device("bluetti", "Bluetti Bluetooth ID", bluetti_device_id, 40);

  WiFiManager wifiManager;

  if (resetWifi){
    wifiManager.resetSettings();
    ESPBluettiSettings defaults;
    wifiConfig = defaults;
    eeprom_saveconfig();
  }

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_password);
  wifiManager.addParameter(&custom_bluetti_device);

  wifiManager.autoConnect("Bluetti_ESP32");
  

  if (shouldSaveConfig) {
     strlcpy(wifiConfig.mqtt_server, custom_mqtt_server.getValue(), 40);
     strlcpy(wifiConfig.mqtt_port, custom_mqtt_port.getValue(), 6);
     strlcpy(wifiConfig.mqtt_username, custom_mqtt_username.getValue(), 40);
     strlcpy(wifiConfig.mqtt_password, custom_mqtt_password.getValue(), 40);
     strlcpy(wifiConfig.bluetti_device_id, custom_bluetti_device.getValue(), 40);
     eeprom_saveconfig();
  }
  */

  WiFi.mode(WIFI_STA);
  WiFi.begin(thessid, thepassword);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  /*
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  */

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("SSID: ");
  Serial.println(thessid);

  /*
  if (MDNS.begin(DEVICE_NAME)) {
    Serial.println("MDNS responder started");
  }

    server.on("/", handleRoot);
    server.on("/rebootDevice", []() {
      server.send(200, "text/plain", "reboot in 2sec");
      delay(2000);
      ESP.restart();
    });
    server.on("/resetConfig", []() {
      server.send(200, "text/plain", "reset Wifi and reboot in 2sec");
      delay(2000);
      initBWifi(true);
  });
  
  

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  */
}

/*
void handleWebserver() {
  server.handleClient();
}

void handleRoot() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html; charset=utf-8", "");
  server.sendContent("<HTML><HEAD><TITLE>device status</TITLE></HEAD><BODY>");
  server.sendContent("<table border='0'>");
  String data = "<tr><td>host:</td><td>" + WiFi.localIP().toString() + "</td><td><a href='http://"+WiFi.localIP().toString()+"/rebootDevice' target='_blank'>reboot this device</a></td></tr>";
  data = data + "<tr><td>SSID:</td><td>" + WiFi.SSID() + "</td><td><a href='http://"+WiFi.localIP().toString()+"/resetConfig' target='_blank'>reset device config</a></td></tr>";
  data = data + "<tr><td>WiFiRSSI:</td><td>" + (String)WiFi.RSSI() + "</td></tr>";
  data = data + "<tr><td>MAC:</td><td>" + WiFi.macAddress() + "</td></tr>";
  data = data + "<tr><td>uptime (ms):</td><td>" + millis() + "</td></tr>";
  data = data + "<tr><td>uptime (h):</td><td>" + millis() / 3600000 + "</td></tr>";
  data = data + "<tr><td>uptime (d):</td><td>" + millis() / 3600000/24 + "</td></tr>";
  data = data + "<tr><td>mqtt server:</td><td>" + wifiConfig.mqtt_server + "</td></tr>";
  data = data + "<tr><td>mqtt port:</td><td>" + wifiConfig.mqtt_port + "</td></tr>";
  data = data + "<tr><td>mqqt connected:</td><td>" + isMQTTconnected() + "</td></tr>";
  data = data + "<tr><td>mqqt last message time:</td><td>" + getLastMQTTMessageTime() + "</td></tr>";
  data = data + "<tr><td>mqqt last devicestate time:</td><td>" + getLastMQTDeviceStateMessageTime() + "</td></tr>";
  data = data + "<tr><td>Bluetti device id:</td><td>" + wifiConfig.bluetti_device_id + "</td></tr>";
  data = data + "<tr><td>BT connected:</td><td>" + isBTconnected() + "</td></tr>";
  data = data + "<tr><td>BT last message time:</td><td>" + getLastBTMessageTime() + "</td></tr>";
  data = data + "<tr><td>BT publishing error:</td><td>" + getPublishErrorCount() + "</td></tr>";

  server.sendContent(data);
  server.sendContent("</table></BODY></HTML>");
  server.client().stop();

}


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
*/
