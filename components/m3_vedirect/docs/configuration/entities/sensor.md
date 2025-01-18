---
title: sensor
parent: Entities
nav_order: 14
---

This entity platform is the default choice when representing read-only [`NUMERIC`](registers#class) registers (i.e. measures).

## Configuration for [`NUMERIC`](registers#class) registers

This is the typical configuration for binding over HEX and TEXT framing layers (The example uses the VE_REG_PANEL_VOLTAGE register which is available in both for MPPT devices):

```yaml
sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Panel voltage"
        register:
          address: 0xEDBB
          text_label: "VPV"
          data_type: UN16
          numeric:
            scale: 0.01
            text_scale: 0.001
            unit: "V"
```

{: .warning}
Note you'd have to properly set the `data_type` in order to correctly decode the raw data for HEX framing. If not set, the component will try to auto-detect the data size (1,2,4 bytes) when receiving the first frame for the register, but it will nevertheless assume the value is an `unsigned` type

- `address`, `data_type`, `scale` and `unit` are all extracted from the official [BlueSolar-HEX-protocol](https://www.victronenergy.com/upload/documents/BlueSolar-HEX-protocol.pdf) for the register.
- `text_label` and `text_scale` (and `unit` too) are extracted from [VE.Direct-Protocol-3.33](https://www.victronenergy.com/upload/documents/VE.Direct-Protocol-3.33.pdf).

{: .highlight}
Setting the unit here will automatically pre-configure also `device_class` and `state_class` according to an internal table. Keep in mind you can freely decide to set those options in the standard EspHome sensor configuration: if you set them, they'll be applied after the internal register configuration so that you can override any aforementioned value (say you don't like the pre-configured `state_class`) or use a `unit_of_measure` which hasn't been normalyzed yet in the vedirect component.

In order to understand the 'defaults', this other minimal configuration should work too (for HEX frames binding):

```yaml
number:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Panel voltage"
        register:
          address: 0xEDBB
```

Here, having not set anything about the register layout, the component will assume:

- `data_type`: unsigned numeric with size automatically inferred when receiving an HEX frame.
- `scale`: defaults to 1 (i.e. exposes the raw values).

The entity will have no `unit_of_measure` nor `device_class` or `state_class` (unless you configure them with the default EspHome config options for `sensor`).

{: .warning}
You shouldn't bind the entity to both HEX and TEXT layers with 'defaults' since it could lead to incoherent decoding: while it works (you can do that), the two frames will likely carry the data with different scales (typically you'd have 0.01 for HEX framing and 0.001 for the TEXT one). With this set, the entity reporting will nevertheless 'oscillate' between the two different raw values (even if the measure is not changing) whenever the frames interleave on the VEDirect channel

## Configuring for TEXT frames only

What if you just want to use TEXT frames? This is an important use case since many times the TEXT protocol is enough to monitor relevant data and you don't want to dig into HEX registers addresses and data formats. Intuitively you could just configure it like:

```yaml
sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Panel voltage"
        register:
          text_label: "VPV"
          numeric:
            text_scale: 0.001
            unit: "V"
```

Again, omitting the `numeric` configuration will 'just work' with some defaults (i.e. no scaling and no unit).

## Configuring for HEX frames only

What if you don't care adding the overhead of TEXT frames parsing to your (micro)controller or if the particular register is not available over the TEXT protocol?

```yaml
sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Panel voltage"
        register:
          address: 0xEDBB
          data_type: UN16
          numeric:
            scale: 0.01
            unit: "V"
```
