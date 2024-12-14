# esphome-victron-vedirect

This [EspHome external component](https://esphome.io/components/external_components) contains a complete implementation to support [Victron VEDirect](https://www.victronenergy.com/upload/documents/VE.Direct-Protocol-3.33.pdf) serial interface. It has parsers for both the HEX and the TEXT protocol data carried over the communication channel and allows to also issue commands in order to configure the Victron device.

In order to use an ESP (both legacy ESP8266 and/or any ESP32 variant) to connect to a VEDirect hardware interface it uses one of the UART buses available in the ESP platform. The code itself is optimized (or can be) to use the minimum amount of resources but, depending on the number of entities configured you might be better off using one of the more capable ESP32 versions so to have more RAM and CPU power for the task.

The details for hardware interfacing can be found online and this repo doesn't provide many clues but you have to basically adapt and connect the VEDirect RX/TX data lines to the ESP uart bus. Now this might be as simple as directly wiring those RX/TX lines to the UART pins of the ESP if the VEDirect device uses 3.3 V operation or, if the interface carries 5V signals, you might be better off using an optocoupler (H11L1 might be needed because of the data transfer rate used in the signaling) or any form of signal level adaptation.

Before using this component I strongly suggest to understand how VEDirect works by deeply checking these (and any other possibly available) docs:
- https://www.victronenergy.com/upload/documents/VE.Direct-Protocol-3.33.pdf
- https://www.victronenergy.com/upload/documents/BMV-7xx-HEX-Protocol.pdf
- https://www.victronenergy.com/upload/documents/BlueSolar-HEX-protocol.pdf
- https://www.victronenergy.com/upload/documents/VE.Direct-HEX-Protocol-Phoenix-Inverter.pdf

The first document being the overall technical description and explaining what is achievable with the TEXT protocol while the others are a more detailed description of the HEX protocol features available among the different classes of devices.

Sadly enough, beside those few documents, Victron doesn't expose a very detailed 'implementation manual' for every device/family, especially the new ones (like the RS families of devices for instance) so, many times you'll have to infer or guess how to interface with these by looking at their most close counterparts for which documentation is available. As an example, I'm using this code and the knowledge from those docs to succesfully (at least partially) interface a Multi RS inverter which contains both MPPTs and INVERTERs features (plus many which are very specific and for which there's no official doc available).
At any rate, for basic sensor readings and configuration those documents are enough since, even among different devices and classes, the configuration registers are pretty almost the same.

## How to use
In order to include the component in your EspHome project use the [instructions](https://esphome.io/components/external_components). Something in the lines:
```yaml
external_components:
  source: github://krahabb/esphome-victron-vedirect
```
There are some sample configurations with a basic explanation of the features for different scenarios:
- [m3_vedirect_basic_example.yaml](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/m3_vedirect_basic_example.yaml)
- [m3_vedirect_minimal_example.yaml](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/m3_vedirect_minimal_example.yaml)
- [m3_vedirect_service_example.yaml](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/m3_vedirect_service_example.yaml)

These samples mostly use the 'auto create' feature in order to automatically create an entity for any register data appearing on the communication channel. This might help to start over but could soon become cumbersome since HEX broadcasted registers might be a lot and the component will create HA entities for any of these.

These other samples show instead usage of static entities definition for different 'flavors'. See [manual configuration](#manual-configuration) for better insights.
- [m3_vedirect_flavor_ALL_example.yaml](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/m3_vedirect_flavor_ALL_example.yaml)
- [m3_vedirect_flavor_BMV_example.yaml](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/m3_vedirect_flavor_BMV_example.yaml)
- [m3_vedirect_flavor_INV_example.yaml](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/m3_vedirect_flavor_INV_example.yaml)
- [m3_vedirect_flavor_MPPT_example.yaml](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/m3_vedirect_flavor_MPPT_example.yaml)


We'll see then how to better configure the device for only the needed entities.

### Manual configuration
Victron devices expose a lot of registers each one carrying either measured data (battery voltage for instance) or a configuration parameter (or some static info like device model, hw & sw revision, and so on). In general, TEXT frames only carry a subset of these (both measures and some info/configuration), while HEX frames can address and carry any register.
Since there are so many registers and most of them are unknown, this component allows you to specify all of the details needed to decode/map any register data to a specific entity (this is an advanced feature and you really need to know how those registers work). This will be covered later.

For simple use cases instead you can use some 'well known' registers definitions which are already embedded in the component (this is related to the 'flavor' configuration). We'll go with some examples for really basic and widely used registers in order to understand this.

Example 1:
Configure some sensors for `battery voltage`, `battery current`, `pv power`.
Here things might be tricky since the component nomenclature tries to use the same conventional names used by Victron docs so let's start to say that (in general) battery voltage in Victron appliances is carried in register `VE_REG_DC_CHANNEL1_VOLTAGE` (and the same reasoning is behind the other entities).
Having said that, in order to setup these sensors you have to configure it like this (for other configuration settings check the samples):
```yaml
m3_vedirect:
  - id: vedirect_0
    uart_id: uart_0
    name: "Victron"
    flavor: [ALL]
    textframe:
      auto_create_entities: false
    hexframe:
      auto_create_entities: false
      ping_timeout: 2min

sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0    
    vedirect_entities:
      - type: DC_CHANNEL1_VOLTAGE
        name: 'Battery voltage'
      - type: DC_CHANNEL1_CURRENT
        name: 'Battery current'
      - type: PANEL_POWER
        name: 'PV power'
```
Using the `type` configuration will automatically set all of the needed info in order to correctly decode the register carrying the battery voltage (DC_CHANNEL1_VOLTAGE) and setup the sensor entity with proper unit, digits and scale. Now, keep in mind the `DC_CHANNEL1_VOLTAGE`-`DC_CHANNEL1_CURRENT`-`PANEL_POWER` registers are defined as `NUMERIC` in 'component terms' and that allows them to be configured as (numeric) sensors (this is useful when inspecting the list of available [pre-defined registers](#register-definitions) published later in order to know which kind of entity supports it)

Let's see another useful 
Example 2:
configure a text_sensor for `DEVICE_STATE`
```yaml
text_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0    
    vedirect_entities:
      - type: DEVICE_STATE
        name: 'Device state (enum)'
```
Register `DEVICE_STATE` (hex address 0x0201) is encoded as an `ENUM` and this component carries (hardcoded in english) the different labels associated with those enum values (according to Victron official nomenclature) so that the text sensor will show you these labels instead of numeric values. This is true for any register defined as `ENUM` so that the text sensor will always try to map the numeric value to a meaningful label. 
If you want to see the underlying numeric value it is possible though: just use a sensor definition instead of a text_sensor:
```yaml
sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - type: DEVICE_STATE
        name: 'Device state (raw)'
```
More: you can map the same register (`DEVICE_STATE`) to more than one entity so that you can see it in different representations (both as a sensor and a text_sensor). The component will dispatch the incoming raw data to all of the entities representing it.

## Register definitions
This table exposes the list of actually pre-defined registers to be used with the 'shortcut' configuration `type`. It is extracted from the source file definitions in [ve_reg_register.h](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/ve_reg_register.h) which is always the 'source of truth' for the component.

<!--BEGIN REG_DEF_TABLE-->
|type|class|r/w|hex address|flavor|
|---|---|---|---|---|
|BLE_MODE|BITMASK|READ_WRITE|0x0090|ANY|
|PRODUCT_ID|UNKNOWN|READ_ONLY|0x0100|ANY|
|APP_VER|UNKNOWN|READ_ONLY|0x0102|ANY|
|SERIAL_NUMBER|STRING|READ_ONLY|0x010A|ANY|
|MODEL_NAME|STRING|READ_ONLY|0x010B|ANY|
|CAPABILITIES|BITMASK|READ_ONLY|0x0140|ANY|
|CAPABILITIES_BLE|BITMASK|READ_ONLY|0x0150|ANY|
|DEVICE_MODE|ENUM|READ_WRITE|0x0200|ANY|
|DEVICE_STATE|ENUM|READ_ONLY|0x0201|ANY|
|DEVICE_OFF_REASON|BITMASK|READ_ONLY|0x0205|ANY|
|DEVICE_OFF_REASON_2|BITMASK|READ_ONLY|0x0207|ANY|
|AC_OUT_VOLTAGE_SETPOINT|NUMERIC|READ_WRITE|0x0230|INV|
|WARNING_REASON|BITMASK|READ_ONLY|0x031C|ANY|
|ALARM_REASON|BITMASK|READ_ONLY|0x031E|ANY|
|ALARM_LOW_VOLTAGE_SET|NUMERIC|READ_WRITE|0x0320|INV|
|ALARM_LOW_VOLTAGE_CLEAR|NUMERIC|READ_WRITE|0x0321|INV|
|RELAY_CONTROL|BOOLEAN|READ_WRITE|0x034E|ANY|
|RELAY_MODE|ENUM|READ_WRITE|0x034F|ANY|
|TTG|NUMERIC|READ_ONLY|0x0FFE|BMV|
|SOC|NUMERIC|READ_ONLY|0x0FFF|BMV|
|AC_OUT_VOLTAGE|NUMERIC|READ_ONLY|0x2200|INV|
|AC_OUT_CURRENT|NUMERIC|READ_ONLY|0x2201|INV|
|AC_OUT_APPARENT_POWER|NUMERIC|READ_ONLY|0x2205|INV|
|SHUTDOWN_LOW_VOLTAGE_SET|NUMERIC|READ_WRITE|0x2210|INV|
|VOLTAGE_RANGE_MIN|NUMERIC|READ_ONLY|0x2211|INV|
|VOLTAGE_RANGE_MAX|NUMERIC|READ_ONLY|0x2212|INV|
|DC_CHANNEL1_VOLTAGE|NUMERIC|READ_ONLY|0xED8D|ANY|
|DC_CHANNEL1_POWER|NUMERIC|READ_ONLY|0xED8E|BMV|
|DC_CHANNEL1_CURRENT|NUMERIC|READ_ONLY|0xED8F|ANY|
|LOAD_OUTPUT_STATE|BOOLEAN|READ_ONLY|0xEDA8|MPPT|
|LOAD_CURRENT|NUMERIC|READ_ONLY|0xEDAD|MPPT|
|MPPT_TRACKER_MODE|ENUM|READ_ONLY|0xEDB3|MPPT|
|PANEL_MAXIMUM_VOLTAGE|NUMERIC|READ_ONLY|0xEDB8|MPPT|
|PANEL_VOLTAGE|NUMERIC|READ_ONLY|0xEDBB|MPPT|
|PANEL_POWER|NUMERIC|READ_ONLY|0xEDBC|MPPT|
|PANEL_CURRENT|NUMERIC|READ_ONLY|0xEDBD|MPPT|
|MAXIMUM_POWER_YESTERDAY|NUMERIC|READ_ONLY|0xEDD0|MPPT|
|YIELD_YESTERDAY|NUMERIC|READ_ONLY|0xEDD1|MPPT|
|MAXIMUM_POWER_TODAY|NUMERIC|READ_ONLY|0xEDD2|MPPT|
|YIELD_TODAY|NUMERIC|READ_ONLY|0xEDD3|MPPT|
|CHARGER_VOLTAGE|NUMERIC|READ_ONLY|0xEDD5|CHG|
|CHARGER_CURRENT|NUMERIC|READ_ONLY|0xEDD7|CHG|
|CHR_ERROR_CODE|ENUM|READ_ONLY|0xEDDA|CHG|
|USER_YIELD|NUMERIC|READ_ONLY|0xEDDC|MPPT|
|SYSTEM_YIELD|NUMERIC|READ_ONLY|0xEDDD|MPPT|
|BAT_TEMPERATURE|NUMERIC|READ_ONLY|0xEDEC|ANY|
|DC_MONITOR_MODE|NUMERIC|READ_ONLY|0xEEB8|BMV71|
|ALARM_BUZZER|BOOLEAN|READ_WRITE|0xEEFC|BMV|
<!--END REG_DEF_TABLE-->


The 'flavor' property is used to conditionally include the corresponding definition(s) in the code. This works as an optimization for code size so that it is possible to decide which of the registers must be defined. It is controlled by the configuration key in the main platform configuration:
```yaml
m3_vedirect:
  - id: vedirect_0
    uart_id: uart_0
    name: "Victron"
    flavor: [ALL] # include all flavors i.e. all of the register definitions
```
If you want to use a particular register definition you have to ensure the corresponding `flavor` is set.

The 'class' and 'r/w' properties are very important and state which kind of data the register is carrying and if it's read only or writable. Different 'classes' and 'r/w' can be supported by different EspHome entity types according to the following table:

|class|r/w|primary entity|
|---|---|---|
|BITMASK|READ_ONLY|binary_sensor(1)|
|BITMASK|READ_WRITE|switch(1)|
|BOOLEAN|READ_ONLY|binary_sensor|
|BOOLEAN|READ_WRITE|switch|
|ENUM|READ_ONLY|text_sensor|
|ENUM|READ_WRITE|select|
|NUMERIC|READ_ONLY|sensor|
|NUMERIC|READ_WRITE|number|
|STRING|READ_ONLY|text_sensor|

(1) in order to use a `binary_sensor` or `switch` for `BITMASK` registers you'd also have to set the configuration property `mask` which, as the name implies, is used to extract/mask the intended bit from the register value (representing a bitmask for the matter).
```yaml
binary_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - type: DEVICE_OFF_REASON_2
        name: 'No input power'
        mask: 1 # Bit 0 of DEVICE_OFF_REASON bitmask
```

Beside these 'primary entity' mappings you can always use a `sensor` entity to represent the raw value as a numeric value even if the register is not marked as `NUMERIC` (though some sign conversion issues might happen since the conversion would be done as an unsigned type whenever the class is not `NUMERIC`). 
Moreover, the `text_sensor` entity too can represent any class data type, generally falling back to report the hex register value 'as is' (if no better conversion is provided by the register definition).

More help and wiki will come.

Good interfacing!

## Notes
This is a 'replica' of the same component as developed on https://github.com/krahabb/esphome. This repository is just an extraction in order to raise its public status.
