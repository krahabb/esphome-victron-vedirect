---
title: Register definitions and Flavors
parent: Configuration
nav_order: 5
---

## Register definitions

This term comes around throughout the documentation to indicate how to configure a register <-> entity mapping but also refers to a 'static' database embedded in code as an array of pre-configured register definitions.
This static array is built using a set of (un)friendly macros defined in [`ve_reg_register.h`](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/ve_reg_register.h) based off Victron official docs and sometimes, some reverse engineering.

These static definitions have two uses actually:

- Auto-creation: when `auto_create_entities` is enabled in component [config]({% link configuration/index.md %}), the component can use the definitions to automatically instantiate the proper entity type when it receives data for a particular register (be it over an HEX or a TEXT frame). If a matching definition is not found, the component will nevertheless create a `text_sensor` which is generally able to render everything in a somewhat meaningful way.
- Compact [configuration](#configuration-using-embedded-register-definitions): as you can see in entities configuration [docs]({% link configuration/entities/index.md %}), the mapping between the entity and the register can be a bit tedious since you have to inspect the docs for register layout and behavior and 'encode' its properties in the EspHome configuration for the entity. Since many registers are pretty common and have wide usage, their layout definition is a repetitive task and that's why the component embeds a database for a somewhat extended set of them (of course it is not exhaustive..)

## Flavors

This term occurs when configuring the [main component]({% link configuration/index.md %}). The config key `flavor` allows you to define a list of flavors which, in the end, determine the groups of static register definitions embedded in the component at build time.
We can think of it as if each of the static register definitions has a flavor attribute so that, when requesting a flavor in component configuration, the register is included in the static database (and so accessible by code) if it has a 'requested' flavor.
You can also think of a `flavor` as a set of related registers implemented in a particular class of devices (for example MPPTs).
Flavors allow to define, at config/build time, which groups of register definitions are linked into the component static database in order to optimize RAM usage by trying including only the sets relevant to the device being interfaced. In fact, it is no point including definitions for registers which are maybe 'Inverter specific' in a project for an MPPT device.

Another important aspect of a flavor, beside grouping registers by device category, is that it has a concept of _hierarchy_ so that when declaring usage of one of them in component configuration, a whole set of _inherited_ flavors will be defined too.
This has to do with the fact that some register groups (i.e. some flavors) are common to multiple (inheriting) flavors.

The following table lists the available flavors and their inheritance

<!--BEGIN FLAVOR_TABLE-->
<!--END FLAVOR_TABLE-->

## Configuration using embedded register definitions

Configuring a register by leveraging the embedded register definitions is just a matter of 'knowing' the name of the register. Here, we'll compare configuration for the same register by using the two methods allowed in this component. The result will be the same.

### Compact configuration:

```yaml
sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Panel voltage"
        register: PANEL_VOLTAGE
```

### Manual configuration:

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

The register `PANEL_VOLTAGE` has `flavor` == `MPPT` so, in order to not incur a config/compilation error you'd have to at least set `flavor: [MPPT]` in component configuration.

The following is the full list of actually defined registers together with their relevant properties:
<!--BEGIN REG_DEF_TABLE-->
<!--END REG_DEF_TABLE-->