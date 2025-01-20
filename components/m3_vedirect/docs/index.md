---
layout: home
title: Home
nav_order: 0
---

> This documentation is _UNDER CONSTRUCTION_ so it is still incomplete and/or under review. Please be patient ;)
> {: .warning}

This EspHome external component contains a complete implementation to support [Victron VEDirect](https://www.victronenergy.com/upload/documents/VE.Direct-Protocol-3.33.pdf) serial interface in an EspHome project.

# Main features:

- TEXT and HEX frame handling
- encode/decode of HEX frames
- compatible with both `ESP8266` and `ESP32` platforms (likely compatible with the EspHome `HOST` platform too)
- multiple components per EspHome node (as far as you have uarts to spare)
- flexible mapping of VEDirect registers to EspHome entities
- includes a 'database' of many common HEX registers layout to ease entities mapping/configuration
- high level interface to directly send HEX frames through either an EspHome Action or in turn an HomeAssistant service
- different layers of customizations/optimizations to reduce memory footprint and/or improve performance

# Quick start

In order to include the component in your EspHome project use the [instructions](https://esphome.io/components/external_components). Something in the lines:

```yaml
external_components:
  source: github://krahabb/esphome-victron-vedirect
```

There are some sample configurations with a basic explanation of the features for different scenarios:

- [m3_vedirect_basic_example.yaml]({% link samples/m3_vedirect_basic_example.yaml %})
- [m3_vedirect_minimal_example.yaml]({% link samples/m3_vedirect_minimal_example.yaml %})
- [m3_vedirect_service_example.yaml]({% link samples/m3_vedirect_service_example.yaml %})

These samples mostly use the 'auto create' feature in order to automatically create an entity for any register data appearing on the communication channel. This might help to start over but could soon become cumbersome since HEX broadcasted registers might be a lot and the component will create HA entities for any of these.

These other samples show instead usage of static entities definition for different 'flavors'.
They're automatically built including all of the definitions for the proposed flavors and might include entities unavailable on your particular device.

- [m3_vedirect_flavor_ALL_example.yaml]({% link samples/m3_vedirect_flavor_ALL_example.yaml %})
- [m3_vedirect_flavor_BMV_example.yaml]({% link samples/m3_vedirect_flavor_BMV_example.yaml %})
- [m3_vedirect_flavor_INV_example.yaml]({% link samples/m3_vedirect_flavor_INV_example.yaml %})
- [m3_vedirect_flavor_MPPT_example.yaml]({% link samples/m3_vedirect_flavor_MPPT_example.yaml %})

Checkout the [Configuration]({% link configuration/index.md %}) pages for details about configuration.

# Hardware

The code uses one of the `uart` resorces available on the platform. In order to connect the uart RX and TX pins to the VEDirect interface follow this [guide](https://www.victronenergy.com/live/vedirect_protocol:faq). When the Victron device port uses 3.3V signals you can wire the VEDirect pins directly to the (ESP) uart pins. When the interface carries 5V signals you should instead adapt those lines to the ESP 3.3V levels and go with a proper opto-coupling device (this is always a good option even when interfacing 3.3V devices). Regarding this matter, I've tested 4N25 devices but they fail to provide the necessary communication speed while H11L1 (and the likes) works like a charm.
