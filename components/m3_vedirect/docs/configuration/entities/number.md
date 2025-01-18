---
title: number
parent: Entities
nav_order: 12
---

This entity platform is the default choice when representing modifiable [`NUMERIC`](registers#class) registers (i.e. configurable numeric values like 'Maximum charge current', 'Bulk voltage' and so on). You cannot set a binding with a TEXT frame label here since it would have no meaning: modifiable registers are not exposed or managed through this layer.

## Configuration for [`NUMERIC`](registers#class) registers

This is the typical configuration needed to bind a configurable register (The example relates to a register available on Phoenix inverters).

```yaml
number:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "AC Output voltage setpoint"
        register:
          address: 0x0230
          data_type: UN16
          numeric:
            scale: 0.01
            unit: "V"
        min_value: 210
        max_value: 240
        step: 0.1
```

{: .warning}
Note you'd have to properly set the `data_type` in order to correctly decode/encode the raw data for HEX framing. If not set, the component will try to auto-detect the data size (1,2,4 bytes) when receiving the first frame for the register, but it will nevertheless assume the value is an `unsigned` type

`address`, `data_type`, `scale` and `unit` are all extracted from the official [VE.Direct-HEX-Protocol-Phoenix-Inverter](https://www.victronenergy.com/upload/documents/VE.Direct-HEX-Protocol-Phoenix-Inverter.pdf) for the register.

In order to understand the 'defaults', this other minimal configuration should work too:

```yaml
number:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "AC Output voltage setpoint"
        register:
          address: 0x0230
```

Here, having not set anything about the register layout, the component will assume:

- `data_type`: unsigned numeric with size automatically inferred when receiving a frame.
- `scale`: defaults to 1 (i.e. exposes/sets the raw values).
- `min`, `max`, `step`: unassigned so you don't have any and things depends on the defaults in the EspHome->HA chain.
