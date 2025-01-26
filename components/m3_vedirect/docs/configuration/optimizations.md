---
title: Optimizations
parent: Configuration
nav_order: 20
---

## Introduction

The code has been designed to be the most _lean_ as possible especially in terms of speed. Nevertheless, the amount of tasks to be carried on is not negligible and of course everything counts. That's why there are some configuration 'patterns' that can produce even more optimized code than the baseline. In general, these optimizations are towards reducing memory footprint.

## Platforms/Components

This is not a 'real' optimization except the fact that the component itself doesn't force any dependency on any other EspHome component except the `uart`. When you setup your entities in EspHome `config.yaml` only the platforms/components for the declared entities will be loaded and linked into the firmware, thus avoiding linking unused parts of code.
The config scenario where you declare and configure the entities should then work as expected. Special care must be taken if you want to leverage the 'entities auto-creation' feature. Here, you have to explicitly 'declare' the platforms in config, else the firmware will not include any entity component. You might go with just declaring the `text_sensor` platform (which is the general purpose choice for raw entity publishing) as a minimal set or just add the entity platforms as needed:

```yaml
binary_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0

sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0

text_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
```

Keep in mind 'entities auto-creation' for data over TEXT frames will only be able to generate entities for `binary_sensor`, `sensor`, `text_sensor`.
In general, if any 'preferred' platform is missing, there's a fallback strategy in place which in the end will try to generate a `text_sensor` (that's why it should be the 'bare-minimum' platform included for any 'auto-creation' scenario).

## Flavors

This has been covered throughout these docs, especially [here]({% link configuration/reg_defs.md %}). When configuring the main component:

```yaml
m3_vedirect:
  - id: vedirect_0
    uart_id: uart_0
    name: "Victron"
    flavor: [ALL]
```

You can decide which group of predefined [register definitions]({% link configuration/reg_defs.md %}) to include in your code.
Every single register definition should take up roughly 11 bytes so that the full set (more than 100 registers) could actually consume around 1 kb of ram (likely more due to alignments).

## Frame parsers

The component standard parser is designed and optimized to manage both HEX and TEXT frames at the same time, supporting also the 'legacy' behavior where TEXT frames could be 'interrupted' by HEX frames (this should not be the case anymore for recent VEDirect firmwares). There are many use-cases where you could only be interested in either TEXT or HEX frame parsing (not both) and this could be used to slightly optimize code size and speed.

This is accomplished in code by defining two pre-processor symbols: `VEDIRECT_USE_HEXFRAME` and `VEDIRECT_USE_TEXTFRAME`. **Keep in mind if none is defined the code will assume as if both are**.
When only one of these symbols is defined, the node will be built with a frame parser optimized for the specified protocol and will 'discard' any data sent by the device in the other framing layer.

Even though EspHome allows you to control the build environment by adding these symbols to your `config`, you'd better off not using this way (explicit config) since the component config generator has some automatic mechanisms to 'infer' what's the best option to use and automatically define those pre-processor symbols for you. These are:

- definition/configuration of `hexframe:` and/or `textframe:` keys in main component config. If you configure any of these keys, its corresponding `USE_XXX` will be defined.
- definition of any `address:` key in a register configuration will set `VEDIRECT_USE_HEXFRAME`
- definition of any `text_label:` key in a register configuration will set `VEDIRECT_USE_TEXTFRAME`
- definition of any [action]({% link configuration/actions_and_triggers.md %}) will set `VEDIRECT_USE_HEXFRAME`

These rules follow the reasoning that, if you're using a feature related to a specific protocol layer then you want to enable parsing for that layer.

{: .highlight}

> - Rules are cumulatively applied so that whenever a rule matches, the corresponding symbol is added to the build options.
> - This logic is still a bit 'fragile' and might change subject to evolving code config/generation syntax and structure

The following examples refer to a node that works the same in both configurations: they'll just use the two differently optimized parsers for the (more or less) same output:

### Example config for HEX frame optimized parsing

```yaml
m3_vedirect:
  - id: vedirect_0
    uart_id: uart_0
    hexframe:
      auto_create_entities: false
      ping_timeout: 2min

text_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Device mode"
        register:
          address: 0x0200
```

### Example config for TEXT frame optimized parsing

```yaml
m3_vedirect:
  - id: vedirect_0
    uart_id: uart_0
    textframe:
      auto_create_entities: false

text_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    vedirect_entities:
      - name: "Device mode"
        register:
          text_label: "MODE"
```

{: .highlight}

> ### Final thoughts
>
> Parser optimization itself is not a big deal and the standard parser should perform well anyway so you shouldnt expect a big change in any performance metric with this or that...  
> It will nevertheless add a bit of speed (some checks are skipped and some states are discarded..) and slightly reduce code size.  
> The main benefit, I guess, would be if you plan to use any feature requiring HEX framing: in this scenario, skipping the TEXT frame parsing might help a lot since TEXT frames are always present on the channel and fully parsing them (instead of discarding..) consumes a bit of power. Keep in mind that, once the channel is in 'HEX mode' (i.e. it starts transmitting HEX data), whenever a register content changes, it should be automatically 'HEX transmitted' by the device according to Victron manuals so that the same updated values are present both in HEX and in TEXT frames: parsing both is just duplicate work.
