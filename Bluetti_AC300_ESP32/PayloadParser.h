#ifndef PAYLOAD_PARSER_H
#define PAYLOAD_PARSER_H
#include "Arduino.h"
#include "DeviceType.h"

#define HEADER_SIZE 4
#define CHECKSUM_SIZE 2

typedef struct __attribute__ ((packed)) {
  uint8_t Request;            // 1 byte
  uint16_t idx;              // 2 bytes  
  uint16_t len;              // 2 bytes  
  uint8_t Buffer[512];        // 512 bytes  
} pageBuffer_t;

uint16_t parse_uint_field(uint8_t data[]);
bool parse_bool_field(uint8_t data[]);
float parse_decimal_field(uint8_t data[], uint8_t scale);
String parse_decimal_array_field(uint8_t data[], uint8_t scale, uint8_t size);
String parse_hex_field(uint8_t data[], uint8_t size, uint16_t addr);
uint64_t parse_serial_field(uint8_t data[]);
float parse_version_field(uint8_t data[]);
String parse_string_field(uint8_t data[]);
String parse_enum_field(uint8_t data[]);

extern int parse_bluetooth_data(uint8_t AddrHighByte, uint8_t AddrLowByte, uint8_t* pData, size_t length, pageBuffer_t *pageBuffer);



#endif
