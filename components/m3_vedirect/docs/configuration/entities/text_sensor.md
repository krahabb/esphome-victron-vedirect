---
title: text_sensor
parent: Entities
nav_order: 16
---

The `text_sensor` platform is able to render all of the register layouts (CLASS) with some special care for `ENUM`, `BITMASK` and obviously `STRING` classes. If no specific class is configured the sensor will by default render the raw HEX payload and/or the raw TEXT payload.

## 'Default' configuration (raw payload)

We'll start analyzing the default configuration (i.e. no register layout specification) since this is the more 'natural' way of considering text_sensor(s) and moreover because this could be a powerful tool to debug/inspect raw registers content. This is similar to what can be done with 'default' sensor(s) which try to render the raw numeric value but sometimes more valuable since it can effectively represent also [`STRING`](registers#class) registers payloads and in general we can consider this to be the 'lowest level of processing' in rendering data.

```yaml
text_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Device mode"
        register:
          address: 0x0200
```

Note you don't have to specify anything since the component will nevertheless extract all of the HEX payload (whatever the length) and just render it with HEX formatting. For example, this `text_sensor` could render `0x04` when the device mode is set to OFF.

{: .warning}

> As for [`sensor`](sensor) in 'default' configuration, You shouldn't bind the entity to both HEX and TEXT layers with 'defaults' since it could lead to incoherent rendering: while it works (you can do that), the two frames will be rendered with different encodings. With this set, the entity reporting will nevertheless 'oscillate' between the two different raw values (even if the underlying value is not changing) whenever the frames interleave on the VEDirect channel

The same register could be binded to its TEXT frame payload this way:

```yaml
text_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Device mode"
        register:
          text_label: "MODE"
```

Now, as the previous warning is pointing out, the rendered content will be the raw text value of the `MODE` TEXT record which for the same state of the previous example will render as `4`.

{: .highlight}

> Having the raw payload reported 'as is' is important and sometimes needed but there are better usage configurations for `text_sensor` when the register has some peculiar layouts. To leverage this feature you'd have to define the register `CLASS` and configure it like explained in the following sections

## Configuration for [`ENUM`](registers#class) registers

We'll use the same example as for [`select`](select) by binding to the VE_REG_DEVICE_MODE register. This register is 'writable' so you'd usually want to use it with the aforementioned platform, nevertheless, you might only be interested in reporting its state without controlling it and you're allowed to do so. This is especially true if you just want to use TEXT frame parsing (more on this in component optimizations) because you don't want to control the Victron device through EspHome.
In some way, you could see the `text_sensor` as the 'read-only' counterpart to `select` (when rendering `ENUM` registers).
Also, using the same register we've used in the 'default' configuration example, you'll see how you can choose between different 'rendering modes' for the same register.

```yaml
text_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Device mode"
        register:
          address: 0x0200
          text_label: "MODE"
          data_type: UN8 # not strictly needed
          enum:
            - 1: "On"
            - 4: "Off"
```

Now, having detailed the mapping between register content and enum labels, the `text_sensor`, when rendering, will try to lookup this mapping and show you the label instead of the raw value. As for the [`select`](select) platform, when a particular mapping is not available, the component will just render the raw value.
In this configuration, binding both framing layers is safe enough to give correct results: even if data have different encodings, the component takes care of decoding those and extract the register value to use in the enum mapping.

## Configuration for [`BITMASK`](registers#class) registers

These registers are a bit of a pain to manage since they carry a lot of data embedded in a single value (i.e. every bit carries a state). There are many options to manage these registers: for example you may want to extract individual (or group of) bits by using a `mask` in [`binary_sensor`](binary_sensor) and report it (them) being `on`/`off`. `text_sensor` instead offers two approaches: the 'default' as seen above could be used to render the raw register value or this other one which works similar to the `ENUM` configuration:

```yaml
text_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Alarm reason"
        register:
          address: 0x031E
          text_label: "AR"
          data_type: UN32 # not strictly needed
          bitmask:
            - 0: "Low battery voltage"
            - 1: "High battery voltage"
            - 2: "Low SOC"
            - 3: "Low Starter Voltage"
            - 4: "High Starter Voltage"
            - 5: "Low Temperature"
            - 6: "High Temperature"
```

Here, you're mapping the individual bits of the register (using the 'position notation') to 'pretty' labels so that the `text_sensor` can use these when rendering the register content. When data comes in, the `text_sensor` will mask every single bit and 'compose' its state by concatenating all the labels of the active bits in the raw value.
An example will better explain this:

- suppose the VE_REG_ALARM_REASON register contains two 'alarms': 'low battery voltage' and 'low soc'
- the register content will be `0x00000005` (i.e. bits `0` and `2` both set)
- `text_sensor` will parse the bits and according to their position lookup through the configured bitmask labels.
- if the bit is set it'll add the label to its own state (auto generating the label in case it's missing from config)
- the output (`text_sensor` state) will be "Low battery voltage,Low SOC"
