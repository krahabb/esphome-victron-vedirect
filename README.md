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
TBD



More help and wiki will come.

Good interfacing!

## Notes
This is a 'replica' of the same component as developed on https://github.com/krahabb/esphome. This repository is just an extraction in order to raise its public status.
