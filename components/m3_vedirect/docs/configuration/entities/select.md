---
title: select
parent: Entities
nav_order: 13
---

This entity platform is the default choice when interacting with modifiable [`ENUM`](registers#class) registers (i.e. the register value is configurable according to a predefined set). You cannot set a binding with a TEXT frame label here since it would have no meaning: modifiable registers are not exposed or managed through this layer.

## Configuration for [`ENUM`](registers#class) registers

This is the typical configuration (The example relates to register `VE_REG_DEVICE_MODE` which is likely available on every device).

```yaml
select:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Device mode"
        register:
          address: 0x0200
          data_type: UN8
          enum:
            - 1: "On"
            - 4: "Off"
```

Knowing the typical configuration layout for other entities, this is straightforward. The `ENUM` class behavior is controlled by the list of items following the `enum` config key. Here we should enumerate the possible (raw) values for the enumeration and assign them a label so that our UI in HomeAssistant would show pretty names instead of raw values. The list must be ordered and doesn't need to be exhaustive:
the component will eventually 'learn' new (unexpected/unconfigured) values when they're sent (as an actual state) from the device. Of course, those new learned values will not have any meaningful label but will just be the string representation of the numeric value. This feature is still a bit 'clunky' since the HA UI doesn't update the list of the select option until the next EspHome node connection.

{: .highlight}

> When using the preset register definitions you can achieve the same result since those definitions contain, for ENUM class registers, the full enumeration labels already encoded (in English language). This is useful but has some caveats:
>
> - Labels are in English (no translations in EspHome).
> - You might not like those labels. I've tried to use the 'minimal' meaningful leabeling according to Victron docs.
> - This embedded label sets are very general in the sense that some devices might only support/need only a few of the items (this is especially true for VE_REG_DEVICE_MODE register) so you'll end up having your HA select UI showing a lot of options while the actual device just supports a few.

As a meaningful example, in order to use the predefined register you could try:

```yaml
select:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Device mode"
        register: DEVICE_MODE
```
