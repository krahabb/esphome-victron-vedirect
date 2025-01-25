---
title: Custom entities
parent: Configuration
nav_order: 3
---

The component, beside allowing binding the entities platforms to VEDirect registers, defines some 'custom entities' which provide some helping or utility features.

## Link connected

This `binary_sensor`, as the name implies, shows the actual link state. It goes `on` whenever it receives a valid frame and goes `off` if no valid frames are received over an internally defined timeout (currently about 5 sec).
Note: having some data on the UART channel doesn't mean the link is active: those data must resolve to a valid HEX or TEXT frame.

```yaml
binary_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    link_connected:
      name: "Link connected"
```

## Raw HEX frame

This is a `text_sensor` used to publish the last received valid HEX frame. It might work as a simple debug tool since it will log, in the corresponding HA sensor, the full history of received HEX frames. As a drawback, this could soon become unmanageable since HA is not very happy when managing a rapid changing history (it might be updated several times per second though). Still could prove to be usuful though.

```yaml
text_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    rawhexframe:
      name: "Raw HEX frame"
```

## Raw TEXT frame

This is a `text_sensor` used to publish the last received valid TEXT frame. It might work as a simple debug tool since it will log, in the corresponding HA sensor, the full history of received TEXT frames. As for the `rawhexframe` tool, this could soon become unmanageable since HA is not very happy when managing a rapid changing history (this entity should update every 1 sec or so).

```yaml
text_sensor:
  - platform: m3_vedirect
    vedirect_id: vedirect_0
    rawtextframe:
      name: "Raw TEXT frame"
```
