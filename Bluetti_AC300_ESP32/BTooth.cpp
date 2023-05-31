#include "BluettiConfig.h"
#include "BTooth.h"
#include "utils.h"
#include "PayloadParser.h"
#include "BWifi.h"
#include "MQTT.h"

int pollTick = 0;


struct command_handle {
  uint8_t AddrHighByte;
  uint8_t AddrLowByte;
  int len;
};

pageBuffer_t pageBuffer;
QueueHandle_t commandHandleQueue;
QueueHandle_t sendQueue;
bt_command_t glob_command_handle;

unsigned long lastBTMessage = 0;

/********************************************/
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }
  
  /********************************************/
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
    #ifdef RELAISMODE
      #ifdef DEBUG
        Serial.println("deactivate relais contact");
      #endif
      digitalWrite(RELAIS_PIN, RELAIS_LOW);
    #endif
  }
};

/********************************************/
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class BluettiAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print(F("BLE Advertised Device found: "));
    Serial.println(advertisedDevice.toString().c_str());

     ESPBluettiSettings settings = get_esp32_bluetti_settings();
    // We have found a device, let us now see if it contains the service we are looking for.
    //if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID) && advertisedDevice.getName().compare(settings.bluetti_device_id)) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID) && (strcmp(advertisedDevice.getName().c_str(),settings.bluetti_device_id)==0)) {
      BLEDevice::getScan()->stop();
      bluettiDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    }
  } 
};

/********************************************/
void initBluetooth(){
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new BluettiAdvertisedDeviceCallbacks());
  /*pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  */
  pBLEScan->setInterval(80);
  pBLEScan->setWindow(0x10);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(40, false);
  
  commandHandleQueue = xQueueCreate( 5, sizeof(bt_command_t ) );
  sendQueue = xQueueCreate( 5, sizeof(bt_command_t) );
  pageBuffer.idx = 0;
  pageBuffer.len = 0;
  
}

/********************************************/
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t len,
  bool isNotify) {

#ifdef DEBUG
  Serial.printf("F01 - Write Response with ;length %d\n", len); //Serial.flush();
  /* pData Debug... */
  for (int i=1; i<=len; i++){
    Serial.printf("%02x", pData[i-1]);//Serial.flush();
    if(i % 2 == 0){
      Serial.print(" ");//Serial.flush();
    }
    if(i % 20 == 0){
      Serial.println();//Serial.flush();
    }
  }
  Serial.println(); //Serial.flush();
#endif

  if(!CMDPending){
    if(xQueueReceive(commandHandleQueue, &glob_command_handle, (TickType_t)10)){
      CMDPending = true;
    }
  }
  if(CMDPending){
    if(!parse_bluetooth_data(glob_command_handle.AddrHighByte, glob_command_handle.AddrLowByte, pData, len, &pageBuffer)){
      CMDPending = false;
    }
  }
}

/********************************************/
bool connectToServer() {
    Serial.print(F("Forming a connection to "));
    Serial.println(bluettiDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(F(" - Created client"));

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(bluettiDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(F(" - Connected to server"));
    for(int i=517; i>=23; i--){
      if(pClient->setMTU(i)) {  ; //set client to request maximum MTU from server (default is 23 otherwise)
        Serial.println();Serial.printf("   setMTU(%d)", i);Serial.println();
        break;
      }
      delay(250);
    }
  
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print(F("Failed to find our service UUID: "));
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(F(" - Found our service"));


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteWriteCharacteristic = pRemoteService->getCharacteristic(WRITE_UUID);
    if (pRemoteWriteCharacteristic == nullptr) {
      Serial.print(F("Failed to find our characteristic UUID: "));
      Serial.println(WRITE_UUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(F(" - Found our Write characteristic"));

        // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteNotifyCharacteristic = pRemoteService->getCharacteristic(NOTIFY_UUID);
    if (pRemoteNotifyCharacteristic == nullptr) {
      Serial.print(F("Failed to find our characteristic UUID: "));
      Serial.println(NOTIFY_UUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(F(" - Found our Notifyite characteristic"));

    // Read the value of the characteristic.
    if(pRemoteWriteCharacteristic->canRead()) {
      std::string value = pRemoteWriteCharacteristic->readValue();
      Serial.print(F("The characteristic value was: "));
      Serial.println(value.c_str());
    }

    if(pRemoteNotifyCharacteristic->canNotify())
      pRemoteNotifyCharacteristic->registerForNotify(notifyCallback);

    connected = true;
     #ifdef RELAISMODE
      #ifdef DEBUG
        Serial.println("activate relais contact");
      #endif
      digitalWrite(RELAIS_PIN, RELAIS_HIGH);
    #endif
    return true;
}

/*******************************************
* neues command aus der sendQueue verschicken
********************************************/
void handleBTCommandQueue(){

  bt_command_t command;
  if(xQueueReceive(sendQueue, &command, 0)) {
      
#ifdef DEBUG
    Serial.print("Write Request FF02 - Value: ");//Serial.flush();
    
    for(int i=0; i<8; i++){
       if ( i % 2 == 0){ Serial.print(" "); };
       Serial.printf("%02x", ((uint8_t*)&command)[i]);//Serial.flush();
    }
    
    Serial.println("");
    //Serial.flush();
#endif
    pRemoteWriteCharacteristic->writeValue((uint8_t*)&command, sizeof(command),true);
 
  }
}

/*******************************************
* neues command in die sendQueue schreiben
********************************************/
void sendBTCommand(bt_command_t command){
  bt_command_t cmd = command;
  xQueueSend(sendQueue, &cmd, 0);
  
}

/********************************************/
void handleBluetooth(){

  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println(F("We are now connected to the Bluetti BLE Server."));
    } else {
      Serial.println(F("We have failed to connect to the server; there is nothin more we will do."));
    }
    doConnect = false;
  }

  if ((millis() - lastBTMessage) > (MAX_DISCONNECTED_TIME_UNTIL_REBOOT * 60000)){ 
    Serial.println(F("BT is disconnected over allowed limit, reboot device"));
    publishErrorTopic(GEN_ERROR, String("BT is disconnected over allowed limit, reboot device"));
    ESP.restart();
  }
  //Serial.printf("CMDPending: %d \n", CMDPending);
  if (connected) {
    if(!CMDPending){
      // poll for device state
      if(millis()-lastBTMessage > BLUETOOTH_QUERY_MESSAGE_DELAY){
  
        /*
        typedef struct __attribute__ ((packed)) {
          uint8_t prefix;            // 1 byte  BT prefix
          uint8_t field_update_cmd;  // 1 byte, Modbus Function code  
          uint8_t AddrHighByte;      // 1 byte, Modbus High Byte Starting Address
          uint8_t AddrLowByte;       // 1 byte, Modbus Low Byte Starting Address
          uint16_t len;              // 2 bytes, Modbus Quantity of Registers
          uint16_t check_sum;        // 2 bytes  
        } bt_command_t;
        */
         bt_command_t command;
         command.prefix = 0x01;
         
         command.field_update_cmd = 0x03;   // Modbus  03 (0x03) Read Holding Registers
         // Register Address = AddrHighByte*16+AddrLowByte
         
         command.AddrHighByte = bluetti_polling_command[pollTick].f_AddrHighByte;
         command.AddrLowByte = bluetti_polling_command[pollTick].f_AddrLowByte;
         command.len = (uint16_t) bluetti_polling_command[pollTick].f_size << 8;  //big endian
         
         command.check_sum = modbus_crc((uint8_t*)&command,6);
#ifdef DEBUG
         Serial.printf("PollCmd%d: Starting Address=%04x, Registers=%d, Checksum: %04x\n", pollTick, (uint16_t)(command.AddrHighByte<<8+command.AddrLowByte), ((command.len & 0xff)+(command.len>>8))*2, command.check_sum);
#endif
         xQueueSend(commandHandleQueue, &command, portMAX_DELAY);
         xQueueSend(sendQueue, &command, portMAX_DELAY);
  
         if (++pollTick > sizeof(bluetti_polling_command)/sizeof(device_field_data_t)-1 ){
             pollTick = 0;
         }
         
        lastBTMessage = millis();
      }
      
      handleBTCommandQueue();
    }
    
  }else if(doScan){
    BLEDevice::getScan()->start(0);  
  }
}

/************************************************************/
bool isBTconnected(){
  return connected;
}

/************************************************************/
unsigned long getLastBTMessageTime(){
    return lastBTMessage;
}
