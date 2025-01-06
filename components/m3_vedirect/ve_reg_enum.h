#pragma once

// clang-format off

#define ENUM_BAT_VOLTAGE_SETTING(ENUM) \
  ENUM(auto_detect, 0x00), ENUM(v12, 12), ENUM(v24, 24), ENUM(v36, 36), ENUM(v48, 48),

// BAT_TYPE enum depends on the device type so we're just providing a 'defective' common definition
#define ENUM_BAT_TYPE(ENUM) \
  ENUM(user, 0xFF), 

#define BITMASK_BLE_MODE(ENUM) \
  ENUM(enabled, 0x00),

#define BITMASK_CAPABILITIES(ENUM) \
  ENUM(load_output, 0), \
  ENUM(rotary_encoder, 1), \
  ENUM(history, 2), \
  ENUM(batterysafe, 3), \
  ENUM(adaptive, 4), \
  ENUM(manual_equalise, 5), \
  ENUM(automatic_equalise, 6), \
  ENUM(storage, 7), \
  ENUM(remote_on_off, 8), \
  ENUM(synchronized_charging, 16), \
  ENUM(relay, 17), \
  ENUM(paygo, 27), \
  ENUM(hybernate, 28),

#define BITMASK_CAPABILITIES_BLE(ENUM) \
  ENUM(ble_mode, 0), ENUM(permanent_off, 1),

#define ENUM_CHR_ERROR_CODE(ENUM) \
  ENUM(no_error, 0), ENUM(battery_temperature_high, 1), ENUM(battery_voltage_high, 2), ENUM(remote_temperature_a, 3), \
  ENUM(remote_temperature_b, 4), ENUM(remote_temperature_c, 5), ENUM(remote_voltage_a, 6), \
  ENUM(remote_voltage_b, 7), ENUM(remote_voltage_c, 8), ENUM(high_ripple, 11), ENUM(battery_temperature_low, 14), \
  ENUM(charger_temperature_high, 17), ENUM(over_current, 18), ENUM(bulk_time, 20), ENUM(current_sensor, 21), \
  ENUM(internal_temperature_a, 22), ENUM(internal_temperature_b, 23), ENUM(fan, 24), ENUM(overheated, 26), \
  ENUM(short_circuit, 27), ENUM(converter_issue, 28), ENUM(over_charge, 29), ENUM(input_voltage, 33), \
  ENUM(input_current, 34), ENUM(input_power, 35), ENUM(input_shutdown_voltage, 38), \
  ENUM(input_shutdown_current, 39), ENUM(input_shutdown_failure, 40), ENUM(inverter_shutdown_41, 41), \
  ENUM(inverter_shutdown_42, 42), ENUM(inverter_shutdown_43, 43), ENUM(inverter_overload, 50), \
  ENUM(inverter_temperature, 51), ENUM(inverter_peak_current, 52), ENUM(inverter_ouput_voltage_a, 53), \
  ENUM(inverter_ouput_voltage_b, 54), ENUM(inverter_self_test_a, 55), ENUM(inverter_self_test_b, 56), \
  ENUM(inverter_ac, 57), ENUM(inverter_self_test_c, 58), ENUM(communication, 65), ENUM(synchronisation, 66), \
  ENUM(bms, 67), ENUM(network_a, 68), ENUM(network_b, 69), ENUM(network_c, 70), ENUM(network_d, 71), \
  ENUM(pv_input_shutdown_80, 80), ENUM(pv_input_shutdown_81, 81), ENUM(pv_input_shutdown_82, 82), \
  ENUM(pv_input_shutdown_83, 83), ENUM(pv_input_shutdown_84, 84), ENUM(pv_input_shutdown_85, 85), \
  ENUM(pv_input_shutdown_86, 86), ENUM(pv_input_shutdown_87, 87), ENUM(cpu_temperature_high, 114), \
  ENUM(calibration_lost, 116), ENUM(invalid_firmware, 117), ENUM(settings_lost, 119), ENUM(tester_fail, 121), \
  ENUM(internal_dc_voltage_a, 200), ENUM(internal_dc_voltage_b, 201), ENUM(self_test, 202), \
  ENUM(internal_supply_voltage_a, 203), ENUM(internal_supply_voltage_b, 205), \
  ENUM(internal_supply_voltage_c, 212), ENUM(internal_supply_voltage_d, 215), ENUM(unknown, 0xFF)

#define ENUM_DEVICE_MODE(ENUM) \
  ENUM(off, 0x00), ENUM(charger_on, 0x01), ENUM(inverter_on, 0x02), ENUM(device_on, 0x03), \
  ENUM(device_off, 0x04), ENUM(eco, 0x05), ENUM(hibernate, 0xFD),

#define BITMASK_DEVICE_OFF_REASON_2(ENUM) \
  ENUM(no_input_power, 0x00), ENUM(physical_power_switch, 0x01), ENUM(soft_power_switch, 0x02), \
  ENUM(remote_input, 0x03), ENUM(internal_alarm, 0x04), ENUM(paygo, 0x05), ENUM(bms_shutdown, 0x06), \
  ENUM(engine, 0x07), ENUM(input_voltage, 0x08), ENUM(battery_temperature, 0x09),

#define ENUM_DEVICE_STATE(ENUM) \
  ENUM(off, 0x00), ENUM(low_power, 0x01), ENUM(fault, 0x02), ENUM(bulk, 0x03), ENUM(absorption, 0x04), \
  ENUM(floating, 0x05), ENUM(storage, 0x06), ENUM(equalize_manual, 0x07), ENUM(passthru, 0x08), \
  ENUM(inverting, 0x09), ENUM(assisting, 0x0A), ENUM(power_supply, 0x0B), ENUM(sustain, 0xF4), \
  ENUM(starting_up, 0xF5), ENUM(repeated_absorption, 0xF6), ENUM(auto_equalize, 0xF7), \
  ENUM(battery_safe, 0xF8), ENUM(external_control, 0xFC), ENUM(unknown, 0xFF),

#define ENUM_MPPT_TRACKER_MODE(ENUM) \
  ENUM(off, 0x00), ENUM(v_i_limited, 0x01), ENUM(mppt, 0x02),

#define ENUM_RELAY_MODE(ENUM) \
  ENUM(warnings, 0), ENUM(remote, 2), ENUM(disabled, 3), ENUM(normal, 4), ENUM(battery_low, 5), \
  ENUM(fan, 6),

#define ENUM_REMOTE_INPUT_MODE_CONFIG(ENUM) \
  ENUM(remote_on_off, 0), ENUM(two_wire_bms, 1),

#define BITMASK_TWO_WIRE_BMS_INPUT_STATE(ENUM) \
  ENUM(two_wire_bms_enabled, 0), ENUM(allow_to_discharge, 1), ENUM(allow_to_charge, 2),

#define BITMASK_WARNING_REASON(ENUM) \
  ENUM(low_battery_voltage, 0x00), ENUM(high_battery_voltage, 0x01), ENUM(low_soc, 0x02), \
  ENUM(low_starter_voltage, 0x03), ENUM(high_starter_voltage, 0x04), ENUM(low_temperature, 0x05), \
  ENUM(high_temperature, 0x06), ENUM(mid_battery_voltage, 0x07), ENUM(overload, 0x08), \
  ENUM(dc_ripple, 0x09), ENUM(low_ac_out_voltage, 0x0A), ENUM(high_ac_out_voltage, 0x0B), \
  ENUM(short_circuit, 0x0C), ENUM(bms_lockout, 0x0D),

// clang-format on