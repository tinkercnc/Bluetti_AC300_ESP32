#include "BluettiConfig.h"
#include "MQTT.h"
#include "PayloadParser.h"
#include "BTooth.h"

//char err_txt[128];
//float v_array[32];
/****************************************/
uint16_t parse_uint_field(uint8_t data[]){
  return ((uint16_t) data[0] << 8 ) | (uint16_t) data[1];
}

/****************************************/
bool parse_bool_field(uint8_t data[]){
  return (data[1]) == 1;
}

/****************************************/
float parse_decimal_field(uint8_t data[], uint8_t scale, uint8_t size){
  uint32_t raw_value = 0;
  int s = (int)size*2-1;

  for(int i=0; i<=s; i++){
    raw_value |= ((uint32_t) data[i] << 8*(s-i) );
  }  
  return (raw_value) / pow(10, scale);
}

/****************************************/
String parse_decimal_array_field(uint8_t data[], uint8_t scale, uint8_t size){
  int32_t raw_value = 0;
  int s = (int)size*2-1;
  float v_array[16];
  String s_array = String("{");

  for(int a=0; a<16; a++){
    for(int i=0; i<=s; i++){
      raw_value |= ((int32_t) data[i] << 8*(s-i) );
    }
    v_array[a] = (raw_value)/pow(10, scale);
    s_array = String(s_array + String(v_array[a],2) + ",");
  }
  s_array = String(s_array + "}");
  return s_array;
}

/****************************************/
String parse_hex_field(uint8_t data[], uint8_t size, uint16_t addr){
  int s = (int)size*2-1;
  String s_array = String(s_array + String(addr, HEX) + ":\n");
  
  for(int i=0; i<=s; i++){
    s_array = String(s_array + String(data[i], HEX));
    if((i+1)%16 == 0) {
      s_array = String(s_array + "\n");
    }else if((i+1)%2 == 0){
      s_array = String(s_array + " ");
    }else{
      s_array = String(s_array + ",");
    }
  }
  return s_array;
}

/****************************************/
float parse_version_field(uint8_t data[]){

  uint16_t low = ((uint16_t) data[0] << 8 ) | (uint16_t) data[1];    
  uint16_t high = ((uint16_t) data[2] << 8) | (uint16_t) data[3];   
  long val = (low ) | (high << 16) ;
  
  return (float) val/100;
}

/****************************************/
uint64_t parse_serial_field(uint8_t data[]){

  uint16_t val1 = ((uint16_t) data[0] << 8 ) | (uint16_t) data[1];
  uint16_t val2 = ((uint16_t) data[2] << 8 ) | (uint16_t) data[3];
  uint16_t val3 = ((uint16_t) data[4] << 8 ) | (uint16_t) data[5];
  uint16_t val4 = ((uint16_t) data[6] << 8 ) | (uint16_t) data[7];
  
  uint64_t sn =  ((((uint64_t) val1) | ((uint64_t) val2 << 16)) | ((uint64_t) val3 << 32)) | ((uint64_t) val4 << 48);
  
  return  sn;
}

/****************************************/
String parse_string_field(uint8_t data[]){
  return String((char*) data);
}

/****************************************/
String parse_enum_field(uint8_t data[]){
    return "";
}

/****************************************/
int parse_bluetooth_data(uint8_t AddrHighByte, uint8_t AddrLowByte, uint8_t* pData, size_t length, pageBuffer_t *pageBuffer){
  uint8_t AddrHigh = AddrHighByte; 
  uint8_t AddrLow = AddrLowByte;
  
#ifdef DEBUG
  Serial.printf("Parser: pData[1] = %02x\n", pData[1]);
  Serial.printf("Parser: pData[2] = %02x\n", pData[2]);
  Serial.printf("AddrHighByte: %02x\nAddrLowByte: %02x \n", 
                  AddrHighByte, AddrLowByte);
#endif

  if(pageBuffer->idx == 0){ //erstes packet
    if(pData[0] == 0x01){
      pageBuffer->Request = pData[1];
      pageBuffer->len = pData[2];
      Serial.printf("len: %d\n", pageBuffer->len); //Serial.flush();
    }else{
      return 0;
     }
  }
  for(int p=0; p<length; p++){
    pageBuffer->Buffer[pageBuffer->idx] = pData[p];
    pageBuffer->idx++;
  }
#ifdef DEBUG
    Serial.printf("pageBuffer->idx: %d\n", pageBuffer->idx); //Serial.flush();
    Serial.printf("pageBuffer->len: %d\n", pageBuffer->len); //Serial.flush();
#endif

  if(pageBuffer->idx < pageBuffer->len+4){  //
    Serial.printf("Message not completed... left bytes: %d\n",  pageBuffer->len - pageBuffer->idx); //Serial.flush();
    //return (pageBuffer->len - pageBuffer->idx+4);
    return -1;
  }

  Serial.printf("Message completed!\n");
  switch(pageBuffer->Request){
    // Hold Register Range Request
    case 0x03:

      for(int i=0; i< sizeof(bluetti_device_state)/sizeof(device_field_data_t); i++){
        
        // filter fields not in range
        if(bluetti_device_state[i].f_AddrHighByte == AddrHigh &&
           bluetti_device_state[i].f_AddrLowByte >= AddrLow &&
           bluetti_device_state[i].f_AddrLowByte <= AddrLow + pageBuffer->len/2 &&
           bluetti_device_state[i].f_AddrLowByte + bluetti_device_state[i].f_size-1 >= AddrLow &&
           bluetti_device_state[i].f_AddrLowByte + bluetti_device_state[i].f_size-1 <= AddrLow + pageBuffer->len/2
        )
        {
          uint8_t data_start = (2* ((int)bluetti_device_state[i].f_AddrLowByte - (int)AddrLowByte)) + HEADER_SIZE;
          uint8_t data_end = (data_start + 2 * bluetti_device_state[i].f_size);
          uint16_t addr = ((uint16_t) bluetti_device_state[i].f_AddrHighByte << 8 ) | (uint16_t) bluetti_device_state[i].f_AddrLowByte;
          
          uint8_t *data_payload_start = pageBuffer->Buffer+data_start-1;
          
#ifdef DEBUG
          Serial.printf("bluetti_device_state[%d]: %d\n", i, bluetti_device_state[i].f_type);//Serial.flush();
#endif                
          switch (bluetti_device_state[i].f_type){
            case UINT_FIELD:
              //Serial.println(bluetti_device_state[i].f_name);//Serial.flush();
              publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(data_payload_start)));
              break;

            case BOOL_FIELD:
              //Serial.println(bluetti_device_state[i].f_name);//Serial.flush();
              publishTopic(bluetti_device_state[i].f_name, String((int)parse_bool_field(data_payload_start)));
              break;

            case DECIMAL_FIELD:
              //Serial.println(bluetti_device_state[i].f_name);//Serial.flush();
              publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(data_payload_start, bluetti_device_state[i].f_scale, bluetti_device_state[i].f_size), 2) );
              break;

            case DECIMAL_ARRAY_FIELD:
              //Serial.println(bluetti_device_state[i].f_name);//Serial.flush();
              
              publishTopic(bluetti_device_state[i].f_name, parse_decimal_array_field(data_payload_start, bluetti_device_state[i].f_scale, bluetti_device_state[i].f_size));
              break;
              
            case SN_FIELD:
              //Serial.println(bluetti_device_state[i].f_name);//Serial.flush();
              char sn[16];
              sprintf(sn, "%lld", parse_serial_field(data_payload_start));
              publishTopic(bluetti_device_state[i].f_name, String(sn));
              break;

            case VERSION_FIELD:
              //Serial.println(bluetti_device_state[i].f_name);//Serial.flush();
              publishTopic(bluetti_device_state[i].f_name, String(parse_version_field(data_payload_start), 2) );    
              break;

            case STRING_FIELD:
              //Serial.println(bluetti_device_state[i].f_name);//Serial.flush();
              publishTopic(bluetti_device_state[i].f_name, parse_string_field(data_payload_start));
              break;

            case ENUM_FIELD:
              //Serial.println(bluetti_device_state[i].f_name);//Serial.flush();
              publishTopic(bluetti_device_state[i].f_name, parse_enum_field(data_payload_start));
              break;

            case HEX_FIELD:
              publishDebugTopic(bluetti_device_state[i].f_name, parse_hex_field(data_payload_start, bluetti_device_state[i].f_size, addr));
              break;
              
            default:
              break;
          }
        }else{
          char err_txt[64];
          /*sprintf(err_txt, "Out_of_Range: HAddr:%02x == %02x; LAddr:%02x >= %02x; len: %d", 
                    bluetti_device_state[i].f_AddrHighByte, 
                    AddrHigh, bluetti_device_state[i].f_AddrLowByte, AddrLow, pageBuffer->len);
          publishErrorTopic(RANGE_ERROR, String(err_txt));
          */
#ifdef DEBUG
          Serial.println("err_txt");
          /*
          Serial.printf("E:bluetti_device_state[%d]: %d\n", i, bluetti_device_state[i].f_type);//Serial.flush();
           
          Serial.printf("E:bluetti_device_state[%d].f_AddrHighByte: %02x == %02x\n", i, bluetti_device_state[i].f_AddrHighByte, AddrHigh);//Serial.flush();
          Serial.printf("E:bluetti_device_state[%d].f_AddrLowByte: %02x >= %02x\n", i, bluetti_device_state[i].f_AddrLowByte, AddrLow);//Serial.flush();
          
          Serial.printf("E:bluetti_device_state[%d].f_size: %04x \n", i, bluetti_device_state[i].f_size);//Serial.flush();
          Serial.printf("E:bt_command_t.len: %04x\n", length);//Serial.flush();
          Serial.printf("E:(AddrLowByte + length)/2: %04\n", (AddrLowByte + length)/2);//Serial.flush();
          Serial.printf(":Ebluetti_device_state[i].f_AddrLowByte + bluetti_device_state[i].f_size-1: %04x\n",
                          bluetti_device_state[i].f_AddrLowByte + bluetti_device_state[i].f_size-1);//Serial.flush();
          Serial.println(); //Serial.flush();
          */
#endif
        }
      }
      break;
    case 0x83:
      char err_txt[64];
      if(pageBuffer->len == 0x02){
        sprintf(err_txt, "ILLEGAL_DATA_ADDRESS: HAddr:%02x; LAddr:%02x", AddrHigh, AddrLow);
        publishErrorTopic(MODBUS_ERROR, String(err_txt));
        Serial.println("ERROR: ILLEGAL DATA ADDRESS");
      }else{
        sprintf(err_txt, "Unknown: len:%02x: HAddr:%02x; LAddr:%02x", pageBuffer->len, AddrHigh, AddrLow);
        publishErrorTopic(GEN_ERROR, String(err_txt));
        Serial.printf("ERROR: Unknown: %02x\n", pageBuffer->len);
      }
      break;
      
    default:
      publishErrorTopic(MODBUS_ERROR, String("Unknown_Request:" + String(pageBuffer->Request, HEX)));
      break;
    
    //CMDPending = false;
  }
  //CMDPending = false;
  pageBuffer->idx = 0;
  pageBuffer->len = 0;
  return 0;
}
