#include "BluettiConfig.h"
#include "MQTT.h"
#include "BWifi.h"
#include "BTooth.h"
#include "utils.h"

#include <PubSubClient.h>

WiFiClient mqttClient;  
PubSubClient client(mqttClient);
int publishErrorCount = 0;
unsigned long lastMQTTMessage = 0;
unsigned long previousDeviceStatePublish = 0;

/************************************************************/
String map_field_name(enum field_names f_name){
   switch(f_name) {
      case DC_OUTPUT_POWER:
        return "dc_output_power";
        break; 
      case AC_OUTPUT_POWER:
        return "ac_output_power";
        break; 
      case DC_OUTPUT_ON:
        return "dc_output_on";
        break; 
      case AC_OUTPUT_ON:
        return "ac_output_on";
        break; 
      case POWER_GENERATION:
        return "power_generation";
        break;       
      case TOTAL_BATTERY_PERCENT:
        return "total_battery_percent";
        break; 
      case DC_INPUT_POWER:
        return "dc_input_power";
        break;
      case AC_INPUT_POWER:
        return "ac_input_power";
        break;
      case SERIAL_NUMBER:
        return "serial_number";
        break;
      case ARM_VERSION:
        return "arm_version";
        break;
      case DSP_VERSION:
        return "dsp_version";
        break;
      case DEVICE_TYPE:
        return "device_type";
        break;
      case UPS_MODE:
        return "ups_mode";
        break;
      case GRID_CHARGE_ON:
        return "GRID_CHARGE_ON";
        break;
      case INTERNAL_AC_VOLTAGE:
        return "internal_ac_voltage";
        break;
      case INTERNAL_CURRENT_1:
        return "internal_current_1";
        break;
      case AC_OUTPUT_MODE:
        return "AC_OUTPUT_MODE";
        break;
      case  INTERNAL_POWER_1:
        return "INTERNAL_POWER_1";
        break;
      case  INTERNAL_AC_FREQ:
        return "INTERNAL_AC_FREQ";
        break;
      case  INTERNAL_CURRENT_2:
        return "INTERNAL_CURRENT_2";
        break;
      case  INTERNAL_POWER_2:
        return "INTERNAL_POWER_2";
        break;
      case  AC_INPUT_VOLTAGE:
        return "AC_INPUT_VOLTAGE";
        break;
      case  INTERNAL_CURRENT_3:
        return "INTERNAL_CURRENT_3";
        break;
      case  INTERNAL_POWER_3:
        return "INTERNAL_POWER_3";
        break;
      case  AC_INPUT_FREQ:
        return "AC_INPUT_FREQ";
        break;
      case  INTERNAL_DC_INPUT_VOLTAGE:
        return "INTERNAL_DC_INPUT_VOLTAGE";
        break;
      case  INTERNAL_DC_INPUT_POWER:
        return "INTERNAL_DC_INPUT_POWER";
        break;
      case  INTERNAL_DC_INPUT_CURRENT:
        return "INTERNAL_DC_INPUT_CURRENT";
        break;
      case PACK_NUM_MAX:
        return "pack_max_num";
        break;
      case PACK_NUM:
        return "PACK_NUM";
        break;
      case PACK_VOLTAGE:
        return "PACK_VOLTAGE";
        break;
      case PACK_BATTERY_PERCENT:
        return "PACK_BATTERY_PERCENT";
        break;
      case CELL_VOLTAGES:
        return "CELL_VOLTAGES";
        break;
      case PACK_BMS_VERSION:
        return "PACK_BMS_VERSION";
        break;
      case  SPLIT_PHASE_ON:
        return "SPLIT_PHASE_ON";
        break;
      case  SPLIT_PHASE_MACHINE_MODE:
        return "SPLIT_PHASE_MACHINE_MODE";
        break;
      case TIME_CONTROL_ON:
        return "TIME_CONTROL_ON";
        break;
      case BATTERY_RANGE_START:
        return "BATTERY_RANGE_START";
        break;
      case BATTERY_RANGE_END:
        return "BATTERY_RANGE_END";
        break;
      case BLUETOOTH_CONNECTED:
        return "BLUETOOTH_CONNECTED";
        break;
      case AUTO_SLEEP_MODE:
        return "AUTO_SLEEP_MODE";
        break;
      case MODBUS_ERROR:
        return "MODBUS_ERROR";
        break;
      case GEN_ERROR:
        return "GEN_ERROR";
        break;
      case RANGE_ERROR:
        return "RANGE_ERROR";
        break;
      case PACK_TEMPERATURE:
        return "PACK_TEMPERATURE";
        break;
      case DEBUG_DATA:
        return "DEBUG_DATA";
        break;
        
      case BAT_UK1:
        return "BAT_UK1";
        break;
      case BAT_UK2:
        return "BAT_UK2";
        break;
      case BAT_UK3:
        return "BAT_UK3";
        break;
      case BAT_UK4:
        return "BAT_UK4";
        break;
      case BAT_UK5:
        return "BAT_UK5";
        break;
      case BAT_UK6:
        return "BAT_UK6";
        break;
        
      default:
        return "unknown";
        break;
   }
}

/************************************************************/
// Callback function
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String topic_path = String(topic);
  
  Serial.print("MQTT Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(" Payload: ");
  String strPayload = String((char * ) payload);
  Serial.println(strPayload);
  
  bt_command_t command;
  command.prefix = 0x01;
  command.field_update_cmd = 0x06;

  for (int i=0; i< sizeof(bluetti_set_command)/sizeof(device_field_data_t); i++){
      if (topic_path.indexOf(map_field_name(bluetti_set_command[i].f_name)) > -1){
            command.AddrHighByte = bluetti_set_command[i].f_AddrHighByte;
            command.AddrLowByte = bluetti_set_command[i].f_AddrLowByte;
      }
  }
  
  command.len = swap_bytes(strPayload.toInt());
  command.check_sum = modbus_crc((uint8_t*)&command,6);
  lastMQTTMessage = millis();
  
  sendBTCommand(command);
}

/************************************************************/
void subscribeTopic(enum field_names field_name) {
  char subscribeTopicBuf[512];
  ESPBluettiSettings settings = get_esp32_bluetti_settings();

  sprintf(subscribeTopicBuf, "bluetti/%s/command/%s", settings.bluetti_device_id, map_field_name(field_name).c_str() );
  client.subscribe(subscribeTopicBuf);
  lastMQTTMessage = millis();

}

/************************************************************/
void publishTopic(enum field_names field_name, String value){
  char publishTopicBuf[1024];

  ESPBluettiSettings settings = get_esp32_bluetti_settings();
  sprintf(publishTopicBuf, "bluetti/%s/state/%s", settings.bluetti_device_id, map_field_name(field_name).c_str() ); 
#ifdef DEBUG
  Serial.printf("%s: %s\n", map_field_name(field_name).c_str(), value);
#endif
  if (!client.publish(publishTopicBuf, value.c_str() )){
    publishErrorCount++;
    Serial.print("MQTT-Publish Error2: ");
    Serial.println(publishErrorCount);
  }else{
    publishErrorCount=0;
  }
  lastMQTTMessage = millis();
 
}

/************************************************************/
void publishDebugTopic(enum field_names field_name, String value){
  char publishTopicBuf[1024];

  ESPBluettiSettings settings = get_esp32_bluetti_settings();
  sprintf(publishTopicBuf, "bluetti/%s/debug/%s", settings.bluetti_device_id, map_field_name(field_name).c_str() ); 
#ifdef DEBUG
  Serial.printf("%s: %s\n", map_field_name(field_name).c_str(), value);
#endif
  if (!client.publish(publishTopicBuf, value.c_str() )){
    publishErrorCount++;
    Serial.print("MQTT-Publish Error2: ");
    Serial.println(publishErrorCount);
  }else{
    publishErrorCount=0;
  }
  lastMQTTMessage = millis();
 
}

/************************************************************/
void publishDeviceState(){
  char publishTopicBuf[1024];

  ESPBluettiSettings settings = get_esp32_bluetti_settings();
  sprintf(publishTopicBuf, "bluetti/%s/state/%s", settings.bluetti_device_id, "device" );
  String value = "{\"IP\":\"" + WiFi.localIP().toString() + "\", \"MAC\":\"" + WiFi.macAddress() + "\", \"Uptime\":" + millis() + "}";
  
#ifdef DEBUG
  Serial.print("publishTopicBuf1: ");
  Serial.println(value);
#endif
  if (!client.publish(publishTopicBuf, value.c_str() )){
    publishErrorCount++;
    Serial.print("MQTT-Publish Error1: ");
    Serial.println(publishErrorCount);
  }else{
    publishErrorCount=0;
  }
  lastMQTTMessage = millis();
  previousDeviceStatePublish = millis();
 
}

/************************************************************/
void publishErrorTopic(enum field_names field_name, String value){
  char publishTopicBuf[1024];

  ESPBluettiSettings settings = get_esp32_bluetti_settings();
  sprintf(publishTopicBuf, "bluetti/%s/error/%s", settings.bluetti_device_id, map_field_name(field_name).c_str() );
#ifdef DEBUG
  Serial.printf("%s: %s\n", map_field_name(field_name).c_str(), value);
#endif
  if (!client.publish(publishTopicBuf, value.c_str() )){
    publishErrorCount++;
    Serial.print("MQTT-Publish Error3: ");
    Serial.println(publishErrorCount);
  }else{
    publishErrorCount=0;
  }
  lastMQTTMessage = millis();
}

/************************************************************/
void initMQTT(){

    enum field_names f_name;
    ESPBluettiSettings settings = get_esp32_bluetti_settings();
    Serial.print("Connecting to MQTT at: ");
    Serial.print(settings.mqtt_server);
    Serial.print(":");
    Serial.println(settings.mqtt_port);
    
    client.setServer(settings.mqtt_server, atoi(settings.mqtt_port));
    client.setCallback(callback);

    bool connect_result;
    const char connect_id[] = "BLUETTI_AC300";
    if (settings.mqtt_username) {
        connect_result = client.connect(connect_id, settings.mqtt_username, settings.mqtt_password);
    } else {
        connect_result = client.connect(connect_id);
    }
    
    if (connect_result) {
        
        Serial.println("Connected to MQTT Server... ");

        // subscribe to topics for commands
        for (int i=0; i< sizeof(bluetti_set_command)/sizeof(device_field_data_t); i++){
          subscribeTopic(bluetti_set_command[i].f_name);
        }
    }

    publishDeviceState();
};

/************************************************************/
void handleMQTT(){
    if ((millis() - lastMQTTMessage) > (MAX_DISCONNECTED_TIME_UNTIL_REBOOT * 60000)){ 
      Serial.println(F("MQTT is disconnected over allowed limit, reboot device"));
      publishErrorTopic(GEN_ERROR, String("MQTT is disconnected over allowed limit, reboot device"));
      ESP.restart();
    }
      
    if ((millis() - previousDeviceStatePublish) > (DEVICE_STATE_UPDATE * 60000)){ // 1 Min.
      publishDeviceState();
    }

    if (!isMQTTconnected() && publishErrorCount > 10){
      Serial.println(F("MQTT lost connection, try to reconnet"));
      client.disconnect();
      lastMQTTMessage=0;
      previousDeviceStatePublish=0;
      publishErrorCount=0;
      initBWifi(false);
      delay(1000);
      initMQTT();
    }
    
    client.loop();
}

/************************************************************/
bool isMQTTconnected(){
    if (client.connected()){
      return true;
    }
    else
    {
      return false;
    }  
}

/************************************************************/
int getPublishErrorCount(){
    return publishErrorCount;
}

/************************************************************/
unsigned long getLastMQTTMessageTime(){
    return lastMQTTMessage;
}

/************************************************************/
unsigned long getLastMQTDeviceStateMessageTime(){
    return previousDeviceStatePublish;
}
