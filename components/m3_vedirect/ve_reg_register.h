/*
  ve_reg_register.h: VE.Direct registers configuration

  Registers are defined using a set of macros which take care of 'propagating'
  these definitions in different code contexts (like generating mock symbols).

  These definitions (if included with the corresponding 'FLAVOR' symbol in code compile/link) will be used to
  auto configure an entity appearing on the channel (auto-create option in configuration.yaml).
  These could also be used to statically configure an entity in configuration.yaml through the 'type' parameter.
  If no 'auto-create' is desired, entities/registers can be manually defined in configuration.yaml
  by using the 'register' parameter and there's no need to add the definition here.

  These macros will then work on the set of symbols/parameters defined here.
  The parameters list depends on the 'CLASS' parameter:

  MACRO(FLAVOR, CLASS, REGISTER_ID, TYPE/LABEL, ACCESS, [DATA_TYPE, UNIT, SCALE, TEXT_SCALE])

  ENUM and BITMASK classes will refer to corresponding 'enum' definitions in ve_reg_enum.h
  The special ENUM_S and BITMASK_S symbols refer to the same but with an additional parameter in order
  to map these registers to a different enum/bitmask definition.

  Be careful when adding new registers to order them by their 'REGISTER_ID' value since the code will
  binary-search this value to find the register definition when 'auto-creating' the registers.

  Register types/labels are set trying to match the official Victron names as much as possible.
  Some special conventions are used to define 'unknown' registers i.e. based off reverse engineering
  field observations.
  For example
    - MACRO(MULTI_RS, NUMERIC, 0xD5C8, UNKNOWN_D5C8, READ_ONLY, uint32_t, UNKNOWN, S_1, S_1)
      is used when the content is observed as unsigned 32-bit integer but the meaning is unknown.
    - MACRO(MULTI_RS, NUMERIC, 0x2215, U_AC_OUT_REAL_POWER, READ_ONLY, int32_t, W, S_1, S_1)
      is used when also the data meaning is inferred (note the U_ prefix) but still there's no official docs.

  WARNING: 'macros are evil' so that any symbol defined through the code-base could interfere
  with those defined here and lead to no compilation or even more subtle issues

*/
#pragma once
#include "ve_reg_flavor.h"
// clang-format off

#define REGISTERS_COMMON(MACRO) \
  MACRO(ANY, BITMASK, 0x0090, BLE_MODE, READ_WRITE, uint8_t) \
  MACRO(ANY, VOID, 0x0100, PRODUCT_ID, CONSTANT) \
  MACRO(ANY, VOID, 0x0102, APP_VER, CONSTANT) \
  MACRO(ANY, STRING, 0x010A, SERIAL_NUMBER, CONSTANT) \
  MACRO(ANY, STRING, 0x010B, MODEL_NAME, CONSTANT) \
  MACRO(ANY, BITMASK, 0x0140, CAPABILITIES, CONSTANT, uint32_t) \
  MACRO(ANY, BITMASK, 0x0150, CAPABILITIES_BLE, CONSTANT, uint32_t) \
  MACRO(ANY, ENUM, 0x0200, DEVICE_MODE, READ_WRITE) \
  MACRO(ANY, ENUM, 0x0201, DEVICE_STATE, READ_ONLY) \
  MACRO(ANY, BITMASK_S, 0x0205, DEVICE_OFF_REASON, READ_ONLY, uint8_t, DEVICE_OFF_REASON_2) \
  MACRO(ANY, BITMASK, 0x0207, DEVICE_OFF_REASON_2, READ_ONLY, uint32_t) \
  MACRO(INV, ENUM_S, 0x0209, INVERTER_DEVICE_STATE, READ_ONLY, DEVICE_STATE) \
  MACRO(CHG, ENUM_S, 0x020A, CHARGER_DEVICE_STATE, READ_ONLY, DEVICE_STATE) \
  MACRO(INV, NUMERIC, 0x0230, AC_OUT_VOLTAGE_SETPOINT, READ_WRITE, uint16_t, V, S_0_01, S_0_01) \
  MACRO(MPPT_RS, NUMERIC, 0x0244, MPPT_TRACKERS, CONSTANT, uint8_t, NONE, S_1, S_1) \
  MACRO(MULTI_RS, NUMERIC, 0x0305, UNKNOWN_0305, READ_ONLY, uint32_t, kWh, S_0_001, S_0_001) \
  MACRO(MULTI_RS, NUMERIC, 0x0310, U_OUTPUT_YIELD, READ_ONLY, uint32_t, kWh, S_0_001, S_0_001) \
  MACRO(MULTI_RS, NUMERIC, 0x0311, U_USER_YIELD, READ_ONLY, uint32_t, kWh, S_0_001, S_0_001) \
  MACRO(ANY, BITMASK, 0x031C, WARNING_REASON, READ_ONLY, uint16_t) \
  MACRO(ANY, BITMASK_S, 0x031E, ALARM_REASON, READ_ONLY, uint16_t, WARNING_REASON) \
  MACRO(INV, NUMERIC, 0x0320, ALARM_LOW_VOLTAGE_SET, READ_WRITE, uint16_t, V, S_0_01, S_0_01) \
  MACRO(INV, NUMERIC, 0x0321, ALARM_LOW_VOLTAGE_CLEAR, READ_WRITE, uint16_t, V, S_0_01, S_0_01) \
  MACRO(ANY, BOOLEAN, 0x034E, RELAY_CONTROL, READ_WRITE) \
  MACRO(ANY, ENUM, 0x034F, RELAY_MODE, READ_WRITE) \
  MACRO(MULTI_RS, NUMERIC, 0x0FFC, UNKNOWN_0FFC, READ_ONLY, uint32_t, UNKNOWN, S_1, S_1) \
  MACRO(BMV, NUMERIC, 0x0FFE, TTG, READ_ONLY, uint16_t, minute, S_1, S_1) \
  MACRO(BMV, NUMERIC, 0x0FFF, SOC, READ_ONLY, uint16_t, SOC_PERCENTAGE, S_0_01, S_0_1) \
  MACRO(MPPT, BOOLEAN, 0x2030, SOLAR_ACTIVITY, READ_ONLY) \
  MACRO(MPPT, NUMERIC, 0x2031, TIME_OF_DAY, READ_ONLY, uint16_t, minute, S_1, S_1) \
  MACRO(INV, NUMERIC, 0x2200, AC_OUT_VOLTAGE, READ_ONLY, int16_t, V, S_0_01, S_0_01) \
  MACRO(INV, NUMERIC, 0x2201, AC_OUT_CURRENT, READ_ONLY, int16_t, A, S_0_1, S_0_1) \
  MACRO(INV, NUMERIC, 0x2205, AC_OUT_APPARENT_POWER, READ_ONLY, int32_t, VA, S_1, S_1) \
  MACRO(INV, NUMERIC, 0x2210, SHUTDOWN_LOW_VOLTAGE_SET, READ_WRITE, uint16_t, V, S_0_01, S_0_01) \
  MACRO(INV, NUMERIC, 0x2211, VOLTAGE_RANGE_MIN, CONSTANT, uint16_t, V, S_0_01, S_0_01) \
  MACRO(INV, NUMERIC, 0x2212, VOLTAGE_RANGE_MAX, CONSTANT, uint16_t, V, S_0_01, S_0_01) \
  MACRO(MULTI_RS, NUMERIC, 0x2213, U_AC_OUT_VOLTAGE, READ_ONLY, int16_t, V, S_0_01, S_0_01) \
  MACRO(MULTI_RS, NUMERIC, 0x2214, U_AC_OUT_CURRENT, READ_ONLY, int16_t, A, S_0_01, S_0_01) \
  MACRO(MULTI_RS, NUMERIC, 0x2215, U_AC_OUT_REAL_POWER, READ_ONLY, int32_t, W, S_1, S_1) \
  MACRO(MULTI_RS, NUMERIC, 0x2216, U_AC_OUT_APPARENT_POWER, READ_ONLY, int32_t, VA, S_1, S_1) \
  MACRO(MULTI_RS, NUMERIC, 0x2250, UNKNOWN_2250, READ_ONLY, int16_t, UNKNOWN, S_1, S_1) \
  MACRO(MULTI_RS, NUMERIC, 0x2251, UNKNOWN_2251, READ_ONLY, int16_t, UNKNOWN, S_1, S_1) \
  MACRO(MPPT_RS, BITMASK, 0xD01F, TWO_WIRE_BMS_INPUT_STATE, READ_ONLY, uint8_t) \
  MACRO(MPPT_RS, ENUM, 0xD0C0, REMOTE_INPUT_MODE_CONFIG, READ_WRITE) \
  MACRO(MULTI_RS, NUMERIC, 0xD3A1, U_AC_OUT_CURRENT_MA, READ_ONLY, int32_t, A, S_0_001, S_0_001) \
  MACRO(MULTI_RS, NUMERIC, 0xD5C8, UNKNOWN_D5C8, READ_ONLY, uint32_t, UNKNOWN, S_1, S_1) \
  MACRO(MULTI_RS, NUMERIC, 0xD5CA, UNKNOWN_D5CA, READ_ONLY, uint32_t, UNKNOWN, S_1, S_1) \
  MACRO(MULTI_RS, NUMERIC, 0xD5CB, UNKNOWN_D5CB, READ_ONLY, uint32_t, UNKNOWN, S_1, S_1) \
  MACRO(MPPT_RS, ENUM_S, 0xECC3, MPPT_TRACKER_MODE_1, READ_ONLY, MPPT_TRACKER_MODE) \
  MACRO(MPPT_RS, NUMERIC, 0xECCB, PANEL_VOLTAGE_1, READ_ONLY, uint16_t, V, S_0_01, S_0_001) \
  MACRO(MPPT_RS, NUMERIC, 0xECCC, PANEL_POWER_1, READ_ONLY, uint32_t, W, S_0_01, S_1) \
  MACRO(MPPT_RS, NUMERIC, 0xECCD, PANEL_CURRENT_1, READ_ONLY, uint16_t, A, S_0_1, S_0_1) \
  MACRO(MPPT_RS, ENUM_S, 0xECD3, MPPT_TRACKER_MODE_2, READ_ONLY, MPPT_TRACKER_MODE) \
  MACRO(MPPT_RS, NUMERIC, 0xECDB, PANEL_VOLTAGE_2, READ_ONLY, uint16_t, V, S_0_01, S_0_001) \
  MACRO(MPPT_RS, NUMERIC, 0xECDC, PANEL_POWER_2, READ_ONLY, uint32_t, W, S_0_01, S_1) \
  MACRO(MPPT_RS, NUMERIC, 0xECDD, PANEL_CURRENT_2, READ_ONLY, uint16_t, A, S_0_1, S_0_1) \
  MACRO(MPPT_RS, ENUM_S, 0xECE3, MPPT_TRACKER_MODE_3, READ_ONLY, MPPT_TRACKER_MODE) \
  MACRO(MPPT_RS, NUMERIC, 0xECEB, PANEL_VOLTAGE_3, READ_ONLY, uint16_t, V, S_0_01, S_0_001) \
  MACRO(MPPT_RS, NUMERIC, 0xECEC, PANEL_POWER_3, READ_ONLY, uint32_t, W, S_0_01, S_1) \
  MACRO(MPPT_RS, NUMERIC, 0xECED, PANEL_CURRENT_3, READ_ONLY, uint16_t, A, S_0_1, S_0_1) \
  MACRO(MPPT_RS, ENUM_S, 0xECF3, MPPT_TRACKER_MODE_4, READ_ONLY, MPPT_TRACKER_MODE) \
  MACRO(MPPT_RS, NUMERIC, 0xECFB, PANEL_VOLTAGE_4, READ_ONLY, uint16_t, V, S_0_01, S_0_001) \
  MACRO(MPPT_RS, NUMERIC, 0xECFC, PANEL_POWER_4, READ_ONLY, uint32_t, W, S_0_01, S_1) \
  MACRO(MPPT_RS, NUMERIC, 0xECFD, PANEL_CURRENT_4, READ_ONLY, uint16_t, A, S_0_1, S_0_1) \
  MACRO(MPPT_RS, NUMERIC, 0xED8B, BATTERY_RIPPLE_VOLTAGE, READ_ONLY, uint16_t, V, S_0_01, S_0_001) \
  MACRO(ANY, NUMERIC, 0xED8D, DC_CHANNEL1_VOLTAGE, READ_ONLY, int16_t, V, S_0_01, S_0_001) \
  MACRO(BMV, NUMERIC, 0xED8E, DC_CHANNEL1_POWER, READ_ONLY, int16_t, W, S_1, S_1) \
  MACRO(ANY, NUMERIC, 0xED8F, DC_CHANNEL1_CURRENT, READ_ONLY, int16_t, A, S_0_1, S_0_001) \
  MACRO(MPPT, BOOLEAN, 0xEDA8, LOAD_OUTPUT_STATE, READ_ONLY) \
  MACRO(MPPT, NUMERIC, 0xEDAD, LOAD_CURRENT, READ_ONLY, uint16_t, A, S_0_1, S_0_001) \
  MACRO(MPPT, ENUM, 0xEDB3, MPPT_TRACKER_MODE, READ_ONLY) \
  MACRO(MPPT, NUMERIC, 0xEDB8, PANEL_MAXIMUM_VOLTAGE, CONSTANT, uint16_t, V, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDBB, PANEL_VOLTAGE, READ_ONLY, uint16_t, V, S_0_01, S_0_001) \
  MACRO(MPPT, NUMERIC, 0xEDBC, PANEL_POWER, READ_ONLY, uint32_t, W, S_0_01, S_1) \
  MACRO(MPPT, NUMERIC, 0xEDBD, PANEL_CURRENT, READ_ONLY, uint16_t, A, S_0_1, S_0_1) \
  MACRO(MPPT_RS, NUMERIC, 0xEDBF, PANEL_MAXIMUM_CURRENT, CONSTANT, uint16_t, A, S_0_1, S_0_1) \
  MACRO(MPPT, NUMERIC, 0xEDCA, VOLTAGE_COMPENSATION, READ_WRITE, uint16_t, V, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDD0, MAXIMUM_POWER_YESTERDAY, READ_ONLY, uint16_t, W, S_1, S_1) \
  MACRO(MPPT, NUMERIC, 0xEDD1, YIELD_YESTERDAY, READ_ONLY, uint16_t, kWh, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDD2, MAXIMUM_POWER_TODAY, READ_ONLY, uint16_t, W, S_1, S_1) \
  MACRO(MPPT, NUMERIC, 0xEDD3, YIELD_TODAY, READ_ONLY, uint16_t, kWh, S_0_01, S_0_01) \
  MACRO(CHG, NUMERIC, 0xEDD5, CHARGER_VOLTAGE, READ_ONLY, uint16_t, V, S_0_01, S_0_001) \
  MACRO(CHG, NUMERIC, 0xEDD7, CHARGER_CURRENT, READ_ONLY, uint16_t, A, S_0_1, S_0_001) \
  MACRO(CHG, ENUM, 0xEDDA, CHR_ERROR_CODE, READ_ONLY) \
  MACRO(CHG, NUMERIC, 0xEDDB, CHR_INTERNAL_TEMPERATURE, READ_ONLY, int16_t, CELSIUS, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDDC, USER_YIELD, READ_ONLY, uint32_t, kWh, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDDD, SYSTEM_YIELD, READ_ONLY, uint32_t, kWh, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDE0, BAT_LOW_TEMP_LEVEL, READ_WRITE, int16_t, CELSIUS, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDE2, REBULK_VOLTAGE_OFFSET, READ_WRITE, uint16_t, V, S_0_01, S_0_01) /* marked as 0xED2E but it seems a typo*/ \
  MACRO(MPPT, NUMERIC, 0xEDE3, EQUALISATION_DURATION, READ_WRITE, uint16_t, HOUR, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDE4, EQUALISATION_CURRENT_LEVEL, READ_WRITE, uint8_t, PERCENTAGE, S_1, S_1) \
  MACRO(MPPT, BOOLEAN, 0xEDE5, AUTO_EQUALISE_STOP_ON_VOLTAGE, READ_WRITE) \
  MACRO(MPPT, NUMERIC, 0xEDE6, LOW_TEMP_CHARGE_CURRENT, READ_WRITE, uint16_t, A, S_0_1, S_0_1) \
  MACRO(MPPT, BOOLEAN, 0xEDE8, BMS_PRESENT, READ_WRITE) \
  MACRO(MPPT, ENUM, 0xEDEA, BAT_VOLTAGE_SETTING, READ_WRITE) \
  MACRO(ANY, NUMERIC, 0xEDEC, BAT_TEMPERATURE, READ_ONLY, uint16_t, CELSIUS, S_0_01, S_1) \
  MACRO(MPPT, NUMERIC, 0xEDEF, BAT_VOLTAGE, READ_ONLY, uint8_t, V, S_1, S_1) \
  MACRO(MPPT, NUMERIC, 0xEDF0, BAT_MAX_CURRENT, READ_WRITE, uint16_t, A, S_0_1, S_0_1) \
  MACRO(MPPT, ENUM, 0xEDF1, BAT_TYPE, READ_WRITE) \
  MACRO(MPPT, NUMERIC, 0xEDF2, BAT_TEMPERATURE_COMPENSATION, READ_WRITE, int16_t, NONE, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDF4, BAT_EQUALISATION_VOLTAGE, READ_WRITE, uint16_t, V, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDF6, BAT_FLOAT_VOLTAGE, READ_WRITE, uint16_t, V, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDF7, BAT_ABSORPTION_VOLTAGE, READ_WRITE, uint16_t, V, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDFB, BAT_ABSORPTION_LIMIT, READ_WRITE, uint16_t, HOUR, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDFC, BAT_BULK_LIMIT, READ_WRITE, uint16_t, HOUR, S_0_01, S_0_01) \
  MACRO(MPPT, NUMERIC, 0xEDFD, AUTOMATIC_EQUALISATION_MODE, READ_WRITE, uint8_t, NONE, S_1, S_1) \
  MACRO(MPPT, BOOLEAN, 0xEDFE, ADAPTIVE_MODE, READ_WRITE) \
  MACRO(BMV71, NUMERIC, 0xEEB8, DC_MONITOR_MODE, READ_ONLY, int16_t, NONE, S_1, S_1) /*might be R/W and it's a signed ENUM*/ \
  MACRO(BMV, BOOLEAN, 0xEEFC, ALARM_BUZZER, READ_WRITE)

// clang-format on