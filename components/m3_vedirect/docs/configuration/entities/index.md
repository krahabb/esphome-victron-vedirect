---
title: Entities
parent: Configuration
nav_order: 0
---

## Introduction

In order to configure entities for the component, we need to understand the characteristics of the VEDirect interface. In general, beside all the low level framing and signals, the communication channel transfers informations about the device through a set of so called ['registers'](Registers) where each is identified by a 16 bit address and represents either a measured value (current, voltage, etc), a state (alarms, warnings, etc), a configuration (bulk voltage, maximum current, etc) or simple static informations like device serial number or firmware version.
These informations are generally carried over HEX frames where you can query/set device registers by address and, for a limited set of measures, states, infos, they're also carried over TEXT frames so that some values/registers are available in both framing layers. The difference is that TEXT frames are automatically broadcasted by the device and you can't decide what's in it. Every 1 second (or so) the device transmits a full TEXT frame containing a (fixed) subset of it's registers (this set has been decided by Victron designers and it is usually enough to monitor the device state in real time). Different device fw versions and/or device families have (or could have) different sets of registers exposed and this is all documented [here](https://www.victronenergy.com/upload/documents/VE.Direct-Protocol-3.33.pdf).

{: .highlight}

> A key point is that HEX frames carry the register address while TEXT frames carry the register content binded to a 'label' (record NAME). This is important when we want to bind our EspHome entities to those registers.

## Baseline example

{: .highlight}

> There are two (almost) interchangeable methods of configuring the binding between a register and an entity:
> - using a preset 'register definition' (REG_DEF in component nomenclature): This is the most compact form and uses a predefined database stored in code (See `flavor` in [main component configuration](Configuration)). This is very compact as you should only set the register name and all the details about the register layout will be preset.
> - manually configuring the register: This is more verbose but allows you to define the register layout/behavior the way you want and surely is needed in case the internal database misses the proper REG_DEF for your register usage.
> The following documentation and examples will focus on the manual configuration in order to grasp the most about the way the component works. Once you got the idea you could anyway use the [preset REG_DEF(s)](/configuration/regdefs) embedded in the various flavors if you find this way better.

As always, it's better to start off with an example and then dig into all of the nuances of entities configuration.
The example uses a `sensor` to configure a `numeric` register since it will likely be the most used overall, being able to represent device measures. We'll then get to know how different entities would better suit different kind of registers.

```yaml
sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Battery current"
        register:
          address: 0xED8F
          text_label: "I"
          data_type: SN16
          numeric:
            scale: 0.1
            text_scale: 0.001
            unit: "A"
```

This sample config will create a numeric `sensor` in EspHome binded to the device register commonly named VE_REG_DC_CHANNEL1_CURRENT which is at address 0xED8F and is usually also carried over TEXT frames with the `I` label. Keep in mind this example will typically work for Victron chargers (MPPTs and not only) and BMVs but, nevertheless, could also be supported by Victron inverters (the docs are pretty 'smoky' for the matter). Let's see how it works:

- `vedirect_entities` (optional - list): is a list of entities for the current platform (sensor) to be binded to the m3_vedirect component specified through `vedirect_id`. Inside the list you can specify all the sensors as needed. This option is the same as for any other platform type so that the preamble configuration is always the same.
- `name` (almost required - string): is the option key inherited from the [sensor platform](https://esphome.io/components/sensor/). Here, beside `name`, you can configure any other base sensor attribute according to EspHome docs.
- `register` (optional - mapping): is a dictionary containing all of the attributes needed to properly setup the register binding to this entity. Depending on entity platform, many, if not all, of the `register` subkeys are optional and will be filled by defaults if not specified. This will allow for a 'quick and dirty' setup if you just want to bind a register to an entity and don't care too much on how the register is configured: the aforementioned defaults might be enough most of the times to 'just work' (just set the `address` and/or the `text_label` subkey(s) and you're done).

  - `address` (optional - numeric): the register address in the VEDirect HEX interface. Check the official docs to get the list of registers for your class of device and see their properties.
  - `text_label` (optional - string): the record LABEL as defined for the VEDirect TEXT protocol specification. Check the official docs to get the list of records/registers for your class of device and see their properties.

  {: .highlight}

  > `address` and `text_label` are both optional meaning you can use either or both in order to bind the entity to the corrisponding protocol frame. Some registers are available in both TEXT and HEX frames and you might want to bind those 2 layers (protocols) of data to the same entity in EspHome (by specifying both keys). This way the entity will be updated whenever each of the data will enter the component. If you just set `address` you're only binding this entity to the data exchanged over the HEX framing. If you just set `text_label` you're only binding this entity to the data coming from the TEXT protocol layer. You can also decide to configure 2 entities for the same register: one binded to the HEX address and the other binded to the text_label (and see if they differ or not!)

  - `data_type` (optional - enum): Here you specify the data size/type of the HEX register frame. Every register has its own structure/layout and this can be checked out in the official docs. If left unset, the component will try to extract the correct size from the received payload but this can lead to misinterpretations of data (for `numeric` registers the data type, beside the length, also sets the signed/unsigned property of the numeric value)
  - `numeric` (optional - enum): This mapping sets the 'CLASS' for the register i.e. its semantics. See [Registers](Registers) for a complete explanation of the concept. Here we're just considering configuration of register VE_REG_DC_CHANNEL1_CURRENT which needs some info in order to be correctly parsed/rendered. We have:

    - `scale` (optional - numeric/enum - default: 1): This is the scale factor used to multiply the incoming raw data from HEX frames (encoded as `data_type`) to the final numeric value for the sensor state. Again, check the Victron docs for cues about registers encoding.
    - `text_scale` (optional - numeric/enum - default: `scale`): Same as the `scale` option but here we're setting the conversion factor for data coming from a TEXT frame. This is usually different than the one used for HEX frames related to the same register.

    {: .highlight}

    > `scale` and `text_scale` allow you to define a numeric (float) constant for the scaling conversion but this cannot be freely choosen. The component restricts the allowed values to a well defined set according to what's usually expected over the data channel (This is due to internal component encodings of config parameters).

    - `unit` (optional - enum): Here you'll set the unit of measure for the entity. This is a 'shortcut' option to what you could achieve by setting the standard `unit_of_measurement` for the sensor component.

    {: .highlight}
    Setting the unit here will automatically pre-configure also `device_class` and `state_class` according to an internal table. Keep in mind you can freely decide to set those 'standard' options: if you set them, they'll be applied after the internal register configuration so that you can override any aforementioned value (say you don't like the pre-configured `state_class`)
