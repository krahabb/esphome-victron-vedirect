#pragma once

// clang-format off

#define BITMASK_BLE_MODE(ENUM) \
  ENUM(ENABLED, 0x00),

#define BITMASK_CAPABILITIES(ENUM) \
  ENUM(LOAD_OUTPUT, 0), \
  ENUM(ROTARY_ENCODER, 1), \
  ENUM(HISTORY, 2), \
  ENUM(BATTERYSAFE, 3), \
  ENUM(ADAPTIVE, 4), \
  ENUM(MANUAL_EQUALISE, 5), \
  ENUM(AUTOMATIC_EQUALISE, 6), \
  ENUM(STORAGE, 7), \
  ENUM(REMOTE_ON_OFF, 8), \
  ENUM(SYNCHRONIZED_CHARGING, 16), \
  ENUM(RELAY, 17), \
  ENUM(PAYGO, 27), \
  ENUM(HYBERNATE, 28), \

#define BITMASK_CAPABILITIES_BLE(ENUM) \
  ENUM(BLE_MODE, 0), ENUM(PERMANENT_OFF, 1),

#define ENUM_CHR_ERROR_CODE(ENUM) \
  ENUM(NO_ERROR, 0), ENUM(BATTERY_TEMPERATURE_HIGH, 1), ENUM(BATTERY_VOLTAGE_HIGH, 2), ENUM(REMOTE_TEMPERATURE_A, 3), \
  ENUM(REMOTE_TEMPERATURE_B, 4), ENUM(REMOTE_TEMPERATURE_C, 5), ENUM(REMOTE_VOLTAGE_A, 6), \
  ENUM(REMOTE_VOLTAGE_B, 7), ENUM(REMOTE_VOLTAGE_C, 8), ENUM(HIGH_RIPPLE, 11), ENUM(BATTERY_TEMPERATURE_LOW, 14), \
  ENUM(CHARGER_TEMPERATURE_HIGH, 17), ENUM(OVER_CURRENT, 18), ENUM(BULK_TIME, 20), ENUM(CURRENT_SENSOR, 21), \
  ENUM(INTERNAL_TEMPERATURE_A, 22), ENUM(INTERNAL_TEMPERATURE_B, 23), ENUM(FAN, 24), ENUM(OVERHEATED, 26), \
  ENUM(SHORT_CIRCUIT, 27), ENUM(CONVERTER_ISSUE, 28), ENUM(OVER_CHARGE, 29), ENUM(INPUT_VOLTAGE, 33), \
  ENUM(INPUT_CURRENT, 34), ENUM(INPUT_POWER, 35), ENUM(INPUT_SHUTDOWN_VOLTAGE, 38), \
  ENUM(INPUT_SHUTDOWN_CURRENT, 39), ENUM(INPUT_SHUTDOWN_FAILURE, 40), ENUM(INVERTER_SHUTDOWN_41, 41), \
  ENUM(INVERTER_SHUTDOWN_42, 42), ENUM(INVERTER_SHUTDOWN_43, 43), ENUM(INVERTER_OVERLOAD, 50), \
  ENUM(INVERTER_TEMPERATURE, 51), ENUM(INVERTER_PEAK_CURRENT, 52), ENUM(INVERTER_OUPUT_VOLTAGE_A, 53), \
  ENUM(INVERTER_OUPUT_VOLTAGE_B, 54), ENUM(INVERTER_SELF_TEST_A, 55), ENUM(INVERTER_SELF_TEST_B, 56), \
  ENUM(INVERTER_AC, 57), ENUM(INVERTER_SELF_TEST_C, 58), ENUM(COMMUNICATION, 65), ENUM(SYNCHRONISATION, 66), \
  ENUM(BMS, 67), ENUM(NETWORK_A, 68), ENUM(NETWORK_B, 69), ENUM(NETWORK_C, 70), ENUM(NETWORK_D, 71), \
  ENUM(PV_INPUT_SHUTDOWN_80, 80), ENUM(PV_INPUT_SHUTDOWN_81, 81), ENUM(PV_INPUT_SHUTDOWN_82, 82), \
  ENUM(PV_INPUT_SHUTDOWN_83, 83), ENUM(PV_INPUT_SHUTDOWN_84, 84), ENUM(PV_INPUT_SHUTDOWN_85, 85), \
  ENUM(PV_INPUT_SHUTDOWN_86, 86), ENUM(PV_INPUT_SHUTDOWN_87, 87), ENUM(CPU_TEMPERATURE_HIGH, 114), \
  ENUM(CALIBRATION_LOST, 116), ENUM(INVALID_FIRMWARE, 117), ENUM(SETTINGS_LOST, 119), ENUM(TESTER_FAIL, 121), \
  ENUM(INTERNAL_DC_VOLTAGE_A, 200), ENUM(INTERNAL_DC_VOLTAGE_B, 201), ENUM(SELF_TEST, 202), \
  ENUM(INTERNAL_SUPPLY_VOLTAGE_A, 203), ENUM(INTERNAL_SUPPLY_VOLTAGE_B, 205), \
  ENUM(INTERNAL_SUPPLY_VOLTAGE_C, 212), ENUM(INTERNAL_SUPPLY_VOLTAGE_D, 215), ENUM(UNKNOWN, 0xFF)

#define ENUM_DEVICE_MODE(ENUM) \
  ENUM(OFF, 0x00), ENUM(CHARGER_ON, 0x01), ENUM(INVERTER_ON, 0x02), ENUM(DEVICE_ON, 0x03), \
  ENUM(DEVICE_OFF, 0x04), ENUM(ECO, 0x05), ENUM(HIBERNATE, 0xFD),

#define BITMASK_DEVICE_OFF_REASON_2(ENUM) \
  ENUM(NO_INPUT_POWER, 0x00), ENUM(PHYSICAL_POWER_SWITCH, 0x01), ENUM(SOFT_POWER_SWITCH, 0x02), \
  ENUM(REMOTE_INPUT, 0x03), ENUM(INTERNAL_ALARM, 0x04), ENUM(PAYGO, 0x05), ENUM(BMS_SHUTDOWN, 0x06), \
  ENUM(ENGINE, 0x07), ENUM(INPUT_VOLTAGE, 0x08), ENUM(BATTERY_TEMPERATURE, 0x09), 

#define ENUM_DEVICE_STATE(ENUM) \
  ENUM(OFF, 0x00), ENUM(LOW_POWER, 0x01), ENUM(FAULT, 0x02), ENUM(BULK, 0x03), ENUM(ABSORPTION, 0x04), \
  ENUM(FLOAT, 0x05), ENUM(STORAGE, 0x06), ENUM(EQUALIZE_MANUAL, 0x07), ENUM(PASSTHRU, 0x08), \
  ENUM(INVERTING, 0x09), ENUM(ASSISTING, 0x0A), ENUM(POWER_SUPPLY, 0x0B), ENUM(SUSTAIN, 0xF4), \
  ENUM(STARTING_UP, 0xF5), ENUM(REPEATED_ABSORPTION, 0xF6), ENUM(AUTO_EQUALIZE, 0xF7), \
  ENUM(BATTERY_SAFE, 0xF8), ENUM(EXTERNAL_CONTROL, 0xFC), ENUM(UNKNOWN, 0xFF),

#define ENUM_MPPT_TRACKER_MODE(ENUM) \
  ENUM(OFF, 0x00), ENUM(V_I_LIMITED, 0x01), ENUM(MPPT, 0x02),

#define ENUM_RELAY_MODE(ENUM) \
  ENUM(WARNINGS, 0), ENUM(REMOTE, 2), ENUM(DISABLED, 3), ENUM(NORMAL, 4), ENUM(BATTERY_LOW, 5), \
  ENUM(FAN, 6),

#define BITMASK_WARNING_REASON(ENUM) \
  ENUM(LOW_BATTERY_VOLTAGE, 0x00), ENUM(HIGH_BATTERY_VOLTAGE, 0x01), ENUM(LOW_SOC, 0x02), \
  ENUM(LOW_STARTER_VOLTAGE, 0x03), ENUM(HIGH_STARTER_VOLTAGE, 0x04), ENUM(LOW_TEMPERATURE, 0x05), \
  ENUM(HIGH_TEMPERATURE, 0x06), ENUM(MID_BATTERY_VOLTAGE, 0x07), ENUM(OVERLOAD, 0x08), \
  ENUM(DC_RIPPLE, 0x09), ENUM(LOW_AC_OUT_VOLTAGE, 0x0A), ENUM(HIGH_AC_OUT_VOLTAGE, 0x0B), \
  ENUM(SHORT_CIRCUIT, 0x0C), ENUM(BMS_LOCKOUT, 0x0D),

// clang-format on