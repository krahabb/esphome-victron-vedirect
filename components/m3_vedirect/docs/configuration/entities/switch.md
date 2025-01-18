---
title: switch
parent: Entities
nav_order: 15
---

This entity platform is the default choice when interacting with configurable [`BOOLEAN`](registers#class) registers. It can also be configured to manage configuration of [`BITMASK`](registers#class) and [`ENUM`](registers#class) classes.

## Configuration for [`BOOLEAN`](registers#class) registers

This is the minimal configuration needed to bind a register which exposes just 0 or 1 in a single byte payload over HEX frames.

```yaml
switch:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Relay"
        register:
          address: 0x034E
          data_type: UN8
          boolean: {} # optional: switch will use BOOLEAN class behavior by default
```

## Configuration for [`BITMASK`](registers#class) registers

{: .highlight}

> This configuration layout is a bit 'exotic' and might not be useful in many use-cases. At any rate, this feature looked funny to me and I've decided to implement it to maybe cover some edge cases.

This configuration allows you to 'control' some bits in a BITMASK register and turn them on/off (all together if the mask contains multiple bits set). It works by masking those bits with the logical AND operator (`&` in c++):

```c
    bool switch_state = (raw_value & mask) ? true : false;
```

When the switch is turned on in HA/EspHome all the bits of the mask will be set in the register preserving the other bits.
On the opposite, when the switch is turned off, all the bits of the mask will be reset, again preserving the other (non masked) bits.

```yaml
switch:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "BLE"
        register:
          address: 0x0090 # register BLE_MODE
          data_type: UN8
          bitmask: {}
        mask: 1 # mask for bit 0
```

Keep in mind the `mask` value is optional and by default it is set to `-1` (convention to set all the bits of a numeric value) meaning every register bit will be toggled.
The example is pretty lame since, according to Victron docs, register 0x0090 currently behaves as a BOOLEAN (i.e. 0->off, 1->on) but is instructive about the flexibility given by configuring the register this way.

## Configuration for [`ENUM`](registers#class) registers

{: .highlight}

> This configuration layout again is a bit 'exotic' but might come handy more then expected. As for the `BITMASK` configuration this feature looked funny too.

This configuration allows you to toggle a register value between 0 and a configurable numeric value (the `mask`). In c++:

```c
    bool switch_state = (raw_value == mask) ? true : false;
```

```yaml
switch:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Charger ON"
        register:
          address: 0x0200 # register VE_REG_DEVICE_MODE
          data_type: UN8
          enum: {}
        mask: 1 # Matches the 'On' state in VE_REG_DEVICE_MODE
      - name: "Charger OFF"
        register:
          address: 0x0200 # register VE_REG_DEVICE_MODE
          data_type: UN8
          enum: {}
        mask: 4 # Matches the 'Off' state in VE_REG_DEVICE_MODE
```

Here, we're using a register whose typical behavior is `ENUM` so that we might be tempted to use a `select` entity to bind (the examples in [select](select) effectively use this).
With this other scenario we're instead using 2 switches to achieve the same result i.e. changing the device mode of operation.
In this example only one of the switches can be `on` at a time since they 'match' different states for the register. Turning on either will send the corrisponding `mask` value to the register so effectively 'selecting' a different mode.

{: .warning}
> In its current implementation, the component sends `0` to the register when being turned off and this might limit usage.
