#ifndef DEVICE_AC300_H
#define DEVICE_AC300_H
#include "Arduino.h"

/* Not implemented yet
enum output_mode {
    STOP = 0,
    INVERTER_OUTPUT = 1,
    BYPASS_OUTPUT_C = 2,
    BYPASS_OUTPUT_D = 3,
    LOAD_MATCHING = 4
};

enum ups_mode {
    CUSTOMIZED = 1,
    PV_PRIORITY = 2,
    STANDARD = 3,
    TIME_CONTROl = 4  
};

enum auto_sleep_mode {
  THIRTY_SECONDS = 2,
  ONE_MINNUTE = 3,
  FIVE_MINUTES = 4,
  NEVER = 5  
};
*/

/* { FIELD_NAME, 
 *  PAGE (AddrHighByte), 
 *  OFFSET {AddrLowByte}, 
 *  SIZE, // Register count (16bit)
 *  SCALE (if scale is needed e.g. decimal value, defaults to 0) , 
 *  ENUM (if data is enum, defaults to 0) , 
 *  FIELD_TYPE }
*/
static device_field_data_t bluetti_device_state[] = {
  /*Page 0x00 Core */
  //{DEBUG_DATA,        0x00, 0x0A, 16, 0, 0, HEX_FIELD },
  
  {DEVICE_TYPE,       0x00, 0x0A, 7, 0, 0, STRING_FIELD},
  {SERIAL_NUMBER,     0x00, 0x11, 4, 0 ,0, SN_FIELD},
  {ARM_VERSION,       0x00, 0x17, 2, 0, 0, VERSION_FIELD},
  {DSP_VERSION,       0x00, 0x19, 2, 0, 0, VERSION_FIELD},
  {DC_INPUT_POWER,    0x00, 0x24, 1, 0, 0, UINT_FIELD},
  {AC_INPUT_POWER,    0x00, 0x25, 1, 0, 0, UINT_FIELD},
  {AC_OUTPUT_POWER,   0x00, 0x26, 1, 0, 0, UINT_FIELD},
  {DC_OUTPUT_POWER,   0x00, 0x27, 1, 0, 0, UINT_FIELD},
  {POWER_GENERATION,  0x00, 0x29, 1, 1, 0, DECIMAL_FIELD},
  {TOTAL_BATTERY_PERCENT, 0x00, 0x2B, 1,0,0, UINT_FIELD},
  {AC_OUTPUT_ON,      0x00, 0x30, 1, 0, 0, BOOL_FIELD},
  {DC_OUTPUT_ON,      0x00, 0x31, 1, 0, 0, BOOL_FIELD},
  //11
  
  // Page 0x00 - Details
  //12
  {AC_OUTPUT_MODE,      0x00, 0x46, 1, 0, 0, DECIMAL_FIELD},
  {INTERNAL_AC_VOLTAGE,   0x00, 0x47, 1, 1, 0, DECIMAL_FIELD},
  {INTERNAL_CURRENT_1,    0x00, 0x48, 1, 0, 0, DECIMAL_FIELD},
  {INTERNAL_POWER_1,      0x00, 0x49, 1, 0, 0, UINT_FIELD},
  {INTERNAL_AC_FREQ,      0x00, 0x4A, 1, 2, 0, DECIMAL_FIELD},
  {INTERNAL_CURRENT_2,    0x00, 0x4B, 1, 0, 0, DECIMAL_FIELD},
  {INTERNAL_POWER_2,      0x00, 0x4C, 1, 0, 0, UINT_FIELD},
  {AC_INPUT_VOLTAGE,      0x00, 0x4D, 1, 1, 0, DECIMAL_FIELD},
  {INTERNAL_CURRENT_3,    0x00, 0x4E, 1, 0, 0, DECIMAL_FIELD},
  {INTERNAL_POWER_3,      0x00, 0x4F, 1, 0, 0, UINT_FIELD},
  {AC_INPUT_FREQ,         0x00, 0x50, 1, 2, 0, DECIMAL_FIELD},
  
  {INTERNAL_DC_INPUT_VOLTAGE, 0x00, 0x56, 1, 0, 0, DECIMAL_FIELD},
  {INTERNAL_DC_INPUT_POWER,   0x00, 0x57, 1, 0, 0, UINT_FIELD},
  {INTERNAL_DC_INPUT_CURRENT, 0x00, 0x58, 1, 0, 0, DECIMAL_FIELD},
  //25
  
  //Page 0x00 Battery Details
  //26
  //{DEBUG_DATA,            0x00, 0x5B, 16, 0, 0, HEX_FIELD },
  {PACK_NUM_MAX,          0x00, 0x5B, 1, 0, 0, UINT_FIELD },
  
  {BAT_UK1,               0x00, 0x5C, 1, 0, 0, UINT_FIELD },
  {BAT_UK2,               0x00, 0x5D, 1, 0, 0, UINT_FIELD },
  {BAT_UK3,               0x00, 0x5E, 1, 0, 0, UINT_FIELD },
  
  {PACK_TEMPERATURE,      0x00, 0x5F, 1, 0, 0, DECIMAL_FIELD },
  {PACK_NUM,              0x00, 0x60, 1, 0, 0, UINT_FIELD },
  {PACK_VOLTAGE,          0x00, 0x62, 2, 0, 0, DECIMAL_FIELD },
  {PACK_BATTERY_PERCENT,  0x00, 0x63, 1, 0, 0, UINT_FIELD },

  {BAT_UK4,               0x00, 0x66, 1, 0, 0, UINT_FIELD },
  {BAT_UK5,               0x00, 0x67, 1, 0, 0, UINT_FIELD },
  {BAT_UK6,               0x00, 0x68, 1, 0, 0, UINT_FIELD },

  //{DEBUG_DATA,            0x00, 0x69, 16, 0, 0, HEX_FIELD },
  {CELL_VOLTAGES,         0x00, 0x69, 16, 0, 0, DECIMAL_ARRAY_FIELD },

  /*{DEBUG_DATA,            0x00, 0x79, 16, 0, 0, HEX_FIELD },
  {DEBUG_DATA,            0x00, 0x89, 16, 0, 0, HEX_FIELD },
  {DEBUG_DATA,            0x00, 0x99, 16, 0, 0, HEX_FIELD },
  {DEBUG_DATA,            0x00, 0xA9, 16, 0, 0, HEX_FIELD },
  {DEBUG_DATA,            0x00, 0xB9, 16, 0, 0, HEX_FIELD },
  {DEBUG_DATA,            0x00, 0xC9, 8, 0, 0, HEX_FIELD },
  {PACK_BMS_VERSION,      0x00, 0xC9, 2, 0, 0, VERSION_FIELD},*/
  // self.struct.add_version_field('pack_bms_version', 0x00, 0xC9)

  /*{DEBUG_DATA,                0x01, 0x0A, 16, 0, 0, HEX_FIELD },
  {DEBUG_DATA,                0x01, 0x1A, 16, 0, 0, HEX_FIELD },
  {DEBUG_DATA,                0x01, 0x2A, 16, 0, 0, HEX_FIELD },*/
  
  //31
  /*Page 0x00 Battery Data 
  {PACK_VOLTAGE, 0x00, 0x62, 1, 2 ,0 ,DECIMAL_FIELD},
  */
  // Page 0x0B - Controls
  //32
  //{DEBUG_DATA,                0x0B, 0xB9, 16, 0, 0, HEX_FIELD },
  
  {UPS_MODE          ,        0x0B, 0xB9, 1, 0, 0, DECIMAL_FIELD},
  {SPLIT_PHASE_ON       ,     0x0B, 0xBC, 1, 0, 0, BOOL_FIELD},
  {SPLIT_PHASE_MACHINE_MODE , 0x0B, 0xBD, 1, 0, 0, DECIMAL_FIELD},
  {PACK_NUM         ,         0x0B, 0xBE, 1, 0, 0, UINT_FIELD},
  {AC_OUTPUT_ON       ,       0x0B, 0xBF, 1, 0, 0, BOOL_FIELD},
  {DC_OUTPUT_ON       ,       0x0B, 0xC0, 1, 0, 0, BOOL_FIELD},
  {GRID_CHARGE_ON       ,     0x0B, 0xC3, 1, 0, 0, BOOL_FIELD},
  {TIME_CONTROL_ON        ,   0x0B, 0xC5, 1, 0, 0, BOOL_FIELD},
  {BATTERY_RANGE_START      , 0x0B, 0xC7, 1, 0, 0, UINT_FIELD},
  {BATTERY_RANGE_END      ,   0x0B, 0xC8, 1, 0, 0, UINT_FIELD},
  {BLUETOOTH_CONNECTED      , 0x0B, 0xDC, 1, 0, 0, BOOL_FIELD},
  
  //{DEBUG_DATA,                0x0B, 0xF0, 16, 0, 0, HEX_FIELD },
  {AUTO_SLEEP_MODE        ,   0x0B, 0xF5, 1, 0, 0, DECIMAL_FIELD},
  //43
};

static device_field_data_t bluetti_set_command[] = {
  /*Page 0x00 Core */
  {AC_OUTPUT_ON,      0x0B, 0xBF, 1, 0, 0, BOOL_FIELD}, 
  {DC_OUTPUT_ON,      0x0B, 0xC0, 1, 0, 0, BOOL_FIELD}
};

static device_field_data_t bluetti_polling_command[] = {
  //{FIELD_UNDEFINED, 0x00, 0x0A, 0x32, 0, 0, TYPE_UNDEFINED},
  {FIELD_UNDEFINED, 0x00, 0x0A, 0x28, 0, 0, TYPE_UNDEFINED},
  
  //{FIELD_UNDEFINED, 0x00, 0x46, 0x15, 0, 0, TYPE_UNDEFINED},
  {FIELD_UNDEFINED, 0x00, 0x46, 0x22, 0, 0, TYPE_UNDEFINED},
  /* CELL_VOLTAGES
  {FIELD_UNDEFINED, 0x00, 0x69, 0x10, 0, 0, TYPE_UNDEFINED},
  */
  // PACK_BMS_VERSION?
  //{FIELD_UNDEFINED, 0x00, 0xC9, 0x08, 0, 0, TYPE_UNDEFINED},
  
  /*{FIELD_UNDEFINED, 0x00, 0xB9, 0x28, 0, 0, TYPE_UNDEFINED}, 
  {FIELD_UNDEFINED, 0x00, 0xe1, 0x28, 0, 0, TYPE_UNDEFINED}, */

  /*{FIELD_UNDEFINED, 0x01, 0x0A, 0x28, 0, 0, TYPE_UNDEFINED},*/
  
  {FIELD_UNDEFINED, 0x0B, 0xB9, 0x3D, 0, 0, TYPE_UNDEFINED},
  //{FIELD_UNDEFINED, 0x0B, 0xF0, 0x10, 0, 0, TYPE_UNDEFINED},
  //{FIELD_UNDEFINED, 0x0B, 0xB9, 0x44, 0, 0, TYPE_UNDEFINED},
};

/*
typedef struct device_field_data {
  enum field_names f_name;
  uint8_t f_AddrHighByte;
  uint8_t f_AddrLowByte;
  int8_t f_size;  // Register count (16bit)
  int8_t f_scale;
  int8_t f_enum;
  enum field_types f_type;
} device_field_data_t;
 */

#endif
