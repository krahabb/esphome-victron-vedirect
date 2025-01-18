---
title: binary_sensor
parent: Entities
nav_order: 11
---

This entity platform is the default choice when representing readable only [`BOOLEAN`](registers#class) registers but it can also be configured to expose data in [`BITMASK`](registers#class) and [`ENUM`](registers#class) classes.

## Configuration for [`BOOLEAN`](registers#class) registers

This is the minimal configuration needed to bind a register which exposes just 0 or 1 in a single byte payload over HEX frames. For this kind of registers, when exposed through the TEXT frame layer, the component will parse the string "ON" in the text payload to match the value.

```yaml
binary_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Load output state"
        register:
          address: 0xEDA8
          text_label: "LOAD"
          boolean: {} # optional: binary_sensor will use BOOLEAN class behavior by default
```

## Configuration for [`BITMASK`](registers#class) registers

This configuration allows you to 'extract' some bits from a BITMASK register to check if any is set. It works by masking those bits with the logical AND operator (`&` in c++):

```c
    bool value = (raw_value & mask) ? true : false;
```

In order to make it work then you have to configure the (bit)mask. The `data_size` is not needed since the component safely extracts the correct length from the HEX (or TEXT) payload.

```yaml
binary_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Switched off"
        register:
          address: 0x0207
          text_label: "OR"
          bitmask: {}
        mask: 14 # mask for bits 1, 2, 3 (2 + 4 + 8) see Victron docs for DEVICE_OFF_REASON(_2) register
```

Keep in mind the `mask` value is optional and by default it is set to `-1` (convention to set all the bits of a numeric value) meaning every register content different from `0` will be reported as `on`. This way it works as a 'generalized' BOOLEAN class register (with any data size).

## Configuration for [`ENUM`](registers#class) registers

This configuration allows you to 'match' the register value against the `mask` parameter. In c++:

```c
    bool value = (raw_value == mask) ? true : false;
```

```yaml
binary_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Device Fault"
        register:
          address: 0x0201 # register VE_REG_DEVICE_STATE
          text_label: "CS"
          enum: {}
        mask: 2 # Matches the 'FAULT' state in VE_REG_DEVICE_STATE
```

Keep in mind the register will be treated as a single byte (this is an internally enforced convention) when you set the class as `ENUM`.
