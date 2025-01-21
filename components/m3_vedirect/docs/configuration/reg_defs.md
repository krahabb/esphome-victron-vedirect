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

| flavor   | inherited flavors                                                   |
| -------- | ------------------------------------------------------------------- |
| ALL      | MULTI_RS, MPPT_BS, MPPT_RS, INV_PHNX, CHG_PHNX, BMV60, BMV70, BMV71 |
| MULTI_RS | INV, MPPT_RS                                                        |
| INV_PHNX | INV                                                                 |
| CHG_PHNX | CHG                                                                 |
| MPPT_BS  | MPPT                                                                |
| MPPT_RS  | MPPT                                                                |
| BMV60    | BMV                                                                 |
| BMV70    | BMV                                                                 |
| BMV71    | BMV                                                                 |
| MPPT     | CHG                                                                 |
| BMV      | ANY                                                                 |
| CHG      | ANY                                                                 |
| INV      | ANY                                                                 |
| ANY      |                                                                     |

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

| register                      | hex address | class   | r/w        | flavor   |
| ----------------------------- | ----------- | ------- | ---------- | -------- |
| BLE_MODE                      | 0x0090      | BITMASK | READ_WRITE | ANY      |
| PRODUCT_ID                    | 0x0100      | VOID    | CONSTANT   | ANY      |
| APP_VER                       | 0x0102      | VOID    | CONSTANT   | ANY      |
| SERIAL_NUMBER                 | 0x010A      | STRING  | CONSTANT   | ANY      |
| MODEL_NAME                    | 0x010B      | STRING  | CONSTANT   | ANY      |
| CAPABILITIES                  | 0x0140      | BITMASK | CONSTANT   | ANY      |
| CAPABILITIES_BLE              | 0x0150      | BITMASK | CONSTANT   | ANY      |
| DEVICE_MODE                   | 0x0200      | ENUM    | READ_WRITE | ANY      |
| DEVICE_STATE                  | 0x0201      | ENUM    | READ_ONLY  | ANY      |
| DEVICE_OFF_REASON             | 0x0205      | BITMASK | READ_ONLY  | ANY      |
| DEVICE_OFF_REASON_2           | 0x0207      | BITMASK | READ_ONLY  | ANY      |
| INVERTER_DEVICE_STATE         | 0x0209      | ENUM    | READ_ONLY  | INV      |
| CHARGER_DEVICE_STATE          | 0x020A      | ENUM    | READ_ONLY  | CHG      |
| AC_OUT_VOLTAGE_SETPOINT       | 0x0230      | NUMERIC | READ_WRITE | INV      |
| MPPT_TRACKERS                 | 0x0244      | NUMERIC | CONSTANT   | MPPT_RS  |
| U_OUTPUT_YIELD                | 0x0310      | NUMERIC | READ_ONLY  | MULTI_RS |
| U_USER_YIELD                  | 0x0311      | NUMERIC | READ_ONLY  | MULTI_RS |
| WARNING_REASON                | 0x031C      | BITMASK | READ_ONLY  | ANY      |
| ALARM_REASON                  | 0x031E      | BITMASK | READ_ONLY  | ANY      |
| ALARM_LOW_VOLTAGE_SET         | 0x0320      | NUMERIC | READ_WRITE | INV      |
| ALARM_LOW_VOLTAGE_CLEAR       | 0x0321      | NUMERIC | READ_WRITE | INV      |
| RELAY_CONTROL                 | 0x034E      | BOOLEAN | READ_WRITE | ANY      |
| RELAY_MODE                    | 0x034F      | ENUM    | READ_WRITE | ANY      |
| TTG                           | 0x0FFE      | NUMERIC | READ_ONLY  | BMV      |
| SOC                           | 0x0FFF      | NUMERIC | READ_ONLY  | BMV      |
| SOLAR_ACTIVITY                | 0x2030      | BOOLEAN | READ_ONLY  | MPPT     |
| TIME_OF_DAY                   | 0x2031      | NUMERIC | READ_ONLY  | MPPT     |
| AC_OUT_VOLTAGE                | 0x2200      | NUMERIC | READ_ONLY  | INV      |
| AC_OUT_CURRENT                | 0x2201      | NUMERIC | READ_ONLY  | INV      |
| AC_OUT_APPARENT_POWER         | 0x2205      | NUMERIC | READ_ONLY  | INV      |
| SHUTDOWN_LOW_VOLTAGE_SET      | 0x2210      | NUMERIC | READ_WRITE | INV      |
| VOLTAGE_RANGE_MIN             | 0x2211      | NUMERIC | CONSTANT   | INV      |
| VOLTAGE_RANGE_MAX             | 0x2212      | NUMERIC | CONSTANT   | INV      |
| U_AC_OUT_VOLTAGE              | 0x2213      | NUMERIC | READ_ONLY  | MULTI_RS |
| U_AC_OUT_CURRENT              | 0x2214      | NUMERIC | READ_ONLY  | MULTI_RS |
| U_AC_OUT_REAL_POWER           | 0x2215      | NUMERIC | READ_ONLY  | MULTI_RS |
| U_AC_OUT_APPARENT_POWER       | 0x2216      | NUMERIC | READ_ONLY  | MULTI_RS |
| TWO_WIRE_BMS_INPUT_STATE      | 0xD01F      | BITMASK | READ_ONLY  | MPPT_RS  |
| REMOTE_INPUT_MODE_CONFIG      | 0xD0C0      | ENUM    | READ_WRITE | MPPT_RS  |
| U_AC_OUT_CURRENT_MA           | 0xD3A1      | NUMERIC | READ_ONLY  | MULTI_RS |
| MPPT_TRACKER_MODE_1           | 0xECC3      | ENUM    | READ_ONLY  | MPPT_RS  |
| PANEL_VOLTAGE_1               | 0xECCB      | NUMERIC | READ_ONLY  | MPPT_RS  |
| PANEL_POWER_1                 | 0xECCC      | NUMERIC | READ_ONLY  | MPPT_RS  |
| PANEL_CURRENT_1               | 0xECCD      | NUMERIC | READ_ONLY  | MPPT_RS  |
| MPPT_TRACKER_MODE_2           | 0xECD3      | ENUM    | READ_ONLY  | MPPT_RS  |
| PANEL_VOLTAGE_2               | 0xECDB      | NUMERIC | READ_ONLY  | MPPT_RS  |
| PANEL_POWER_2                 | 0xECDC      | NUMERIC | READ_ONLY  | MPPT_RS  |
| PANEL_CURRENT_2               | 0xECDD      | NUMERIC | READ_ONLY  | MPPT_RS  |
| MPPT_TRACKER_MODE_3           | 0xECE3      | ENUM    | READ_ONLY  | MPPT_RS  |
| PANEL_VOLTAGE_3               | 0xECEB      | NUMERIC | READ_ONLY  | MPPT_RS  |
| PANEL_POWER_3                 | 0xECEC      | NUMERIC | READ_ONLY  | MPPT_RS  |
| PANEL_CURRENT_3               | 0xECED      | NUMERIC | READ_ONLY  | MPPT_RS  |
| MPPT_TRACKER_MODE_4           | 0xECF3      | ENUM    | READ_ONLY  | MPPT_RS  |
| PANEL_VOLTAGE_4               | 0xECFB      | NUMERIC | READ_ONLY  | MPPT_RS  |
| PANEL_POWER_4                 | 0xECFC      | NUMERIC | READ_ONLY  | MPPT_RS  |
| PANEL_CURRENT_4               | 0xECFD      | NUMERIC | READ_ONLY  | MPPT_RS  |
| BATTERY_RIPPLE_VOLTAGE        | 0xED8B      | NUMERIC | READ_ONLY  | MPPT_RS  |
| DC_CHANNEL1_VOLTAGE           | 0xED8D      | NUMERIC | READ_ONLY  | ANY      |
| DC_CHANNEL1_POWER             | 0xED8E      | NUMERIC | READ_ONLY  | BMV      |
| DC_CHANNEL1_CURRENT           | 0xED8F      | NUMERIC | READ_ONLY  | ANY      |
| LOAD_OUTPUT_STATE             | 0xEDA8      | BOOLEAN | READ_ONLY  | MPPT     |
| LOAD_CURRENT                  | 0xEDAD      | NUMERIC | READ_ONLY  | MPPT     |
| MPPT_TRACKER_MODE             | 0xEDB3      | ENUM    | READ_ONLY  | MPPT     |
| PANEL_MAXIMUM_VOLTAGE         | 0xEDB8      | NUMERIC | CONSTANT   | MPPT     |
| PANEL_VOLTAGE                 | 0xEDBB      | NUMERIC | READ_ONLY  | MPPT     |
| PANEL_POWER                   | 0xEDBC      | NUMERIC | READ_ONLY  | MPPT     |
| PANEL_CURRENT                 | 0xEDBD      | NUMERIC | READ_ONLY  | MPPT     |
| PANEL_MAXIMUM_CURRENT         | 0xEDBF      | NUMERIC | CONSTANT   | MPPT_RS  |
| VOLTAGE_COMPENSATION          | 0xEDCA      | NUMERIC | READ_WRITE | MPPT     |
| MAXIMUM_POWER_YESTERDAY       | 0xEDD0      | NUMERIC | READ_ONLY  | MPPT     |
| YIELD_YESTERDAY               | 0xEDD1      | NUMERIC | READ_ONLY  | MPPT     |
| MAXIMUM_POWER_TODAY           | 0xEDD2      | NUMERIC | READ_ONLY  | MPPT     |
| YIELD_TODAY                   | 0xEDD3      | NUMERIC | READ_ONLY  | MPPT     |
| CHARGER_VOLTAGE               | 0xEDD5      | NUMERIC | READ_ONLY  | CHG      |
| CHARGER_CURRENT               | 0xEDD7      | NUMERIC | READ_ONLY  | CHG      |
| CHR_ERROR_CODE                | 0xEDDA      | ENUM    | READ_ONLY  | CHG      |
| CHR_INTERNAL_TEMPERATURE      | 0xEDDB      | NUMERIC | READ_ONLY  | CHG      |
| USER_YIELD                    | 0xEDDC      | NUMERIC | READ_ONLY  | MPPT     |
| SYSTEM_YIELD                  | 0xEDDD      | NUMERIC | READ_ONLY  | MPPT     |
| BAT_LOW_TEMP_LEVEL            | 0xEDE0      | NUMERIC | READ_WRITE | MPPT     |
| REBULK_VOLTAGE_OFFSET         | 0xEDE2      | NUMERIC | READ_WRITE | MPPT     |
| EQUALISATION_DURATION         | 0xEDE3      | NUMERIC | READ_WRITE | MPPT     |
| EQUALISATION_CURRENT_LEVEL    | 0xEDE4      | NUMERIC | READ_WRITE | MPPT     |
| AUTO_EQUALISE_STOP_ON_VOLTAGE | 0xEDE5      | BOOLEAN | READ_WRITE | MPPT     |
| LOW_TEMP_CHARGE_CURRENT       | 0xEDE6      | NUMERIC | READ_WRITE | MPPT     |
| BMS_PRESENT                   | 0xEDE8      | BOOLEAN | READ_WRITE | MPPT     |
| BAT_VOLTAGE_SETTING           | 0xEDEA      | ENUM    | READ_WRITE | MPPT     |
| BAT_TEMPERATURE               | 0xEDEC      | NUMERIC | READ_ONLY  | ANY      |
| BAT_VOLTAGE                   | 0xEDEF      | NUMERIC | READ_ONLY  | MPPT     |
| BAT_MAX_CURRENT               | 0xEDF0      | NUMERIC | READ_WRITE | MPPT     |
| BAT_TYPE                      | 0xEDF1      | ENUM    | READ_WRITE | MPPT     |
| BAT_TEMPERATURE_COMPENSATION  | 0xEDF2      | NUMERIC | READ_WRITE | MPPT     |
| BAT_EQUALISATION_VOLTAGE      | 0xEDF4      | NUMERIC | READ_WRITE | MPPT     |
| BAT_FLOAT_VOLTAGE             | 0xEDF6      | NUMERIC | READ_WRITE | MPPT     |
| BAT_ABSORPTION_VOLTAGE        | 0xEDF7      | NUMERIC | READ_WRITE | MPPT     |
| BAT_ABSORPTION_LIMIT          | 0xEDFB      | NUMERIC | READ_WRITE | MPPT     |
| BAT_BULK_LIMIT                | 0xEDFC      | NUMERIC | READ_WRITE | MPPT     |
| AUTOMATIC_EQUALISATION_MODE   | 0xEDFD      | NUMERIC | READ_WRITE | MPPT     |
| ADAPTIVE_MODE                 | 0xEDFE      | BOOLEAN | READ_WRITE | MPPT     |
| DC_MONITOR_MODE               | 0xEEB8      | NUMERIC | READ_ONLY  | BMV71    |
| ALARM_BUZZER                  | 0xEEFC      | BOOLEAN | READ_WRITE | BMV      |

<!--END REG_DEF_TABLE-->