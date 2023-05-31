#ifndef BTOOTH_H
#define BTOOTH_H

#include "Arduino.h"
#include "BLEDevice.h"

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static boolean CMDPending = false;
static BLERemoteCharacteristic* pRemoteWriteCharacteristic;
static BLERemoteCharacteristic* pRemoteNotifyCharacteristic;
static BLEAdvertisedDevice* bluettiDevice;

typedef struct __attribute__ ((packed)) {
  uint8_t prefix;            // 1 byte  BT prefix
  uint8_t field_update_cmd;  // 1 byte, Modbus Function code  
  uint8_t AddrHighByte;              // 1 byte, Modbus High Byte Starting Address
  uint8_t AddrLowByte;            // 1 byte, Modbus Low Byte Starting Address
  uint16_t len;              // 2 bytes, Modbus Quantity of Registers
  uint16_t check_sum;        // 2 bytes  
} bt_command_t;

// The remote Bluetti service we wish to connect to.
static BLEUUID serviceUUID("0000ff00-0000-1000-8000-00805f9b34fb");

// The characteristics of Bluetti Devices
static BLEUUID    WRITE_UUID("0000ff02-0000-1000-8000-00805f9b34fb");
static BLEUUID    NOTIFY_UUID("0000ff01-0000-1000-8000-00805f9b34fb");

extern void initBluetooth();
extern void handleBluetooth();
bool connectToServer();
extern void handleBTCommandQueue();
extern void sendBTCommand(bt_command_t command);
extern bool isBTconnected();
extern unsigned long getLastBTMessageTime();

#endif
