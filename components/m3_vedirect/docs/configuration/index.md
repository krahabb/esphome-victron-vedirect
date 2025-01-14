---
title: Configuration
nav_order: 5
---

Let's see in detail how to configure the main component and related entities:

```yaml
external_components:
  source: github://krahabb/esphome-victron-vedirect

uart:
  - id: uart_0
    tx_pin: GPIO4
    rx_pin: GPIO5
    baud_rate: 19200
    rx_buffer_size: 256

m3_vedirect:
  - id: vedirect_0
    uart_id: uart_0
    name: "Victron"
    flavor: [ALL]
    textframe:
      auto_create_entities: true
    hexframe:
      auto_create_entities: true
      ping_timeout: 2min
```

This is a simple snippet where we're going to setup the needed `uart` component which, depending on the platform (ESP8266,ESP32,HOST) might have different options.
The `m3_vedirect` component has this configuration options available:
- `id` (required): used to refer to this component when configuring entities for EspHome.
- `uart_id` (required): the `id` of the uart component to be linked to this.
- `name` (optional - string): This string is prepended to the entity name when an entity is dynamically built by the component (see `auto_create_entities`). Could be left empty, but if you have more than one m3_vedirect component per EspHome node you could use this to distinguish auto created entities related to different VEDirect devices.
- `flavor` (optional - list - default: [ALL]): The concept of 'flavor' is strictly related to 'register definitions' which are linked (embedded) in the component code. These register definitions offer a synthetic grammar of register behavior and are used to correctly setup entities. Since the list of these definitions might grow huge (depending on component development) the memory footprint could be large and, depending on your specific use-case, this list might be a waste of memory if you're not using it. That's why these register definitions are grouped in 'flavors' so that you can selectively enable them by leveraging this configuration option. This way, the compiled/linked firmware can be shrinked to only contain a subset of the whole list. 'Flavors' are strictly defined in code and you can choose from a set of possible options. By default (i.e. if no `flavor` key is set), all the flavors will be included while, if you want to completely 'reset' the list of register definitions so to free up the maximum amount of memory you'd have to set this option to an empty list -> `flavor`: [].
- `textframe` (optional - mapping): Configures behavior for TEXT frames handling
  - `auto_create_entities` (optional - default: true): This options configures the component to automatically build an entity for every TEXT RECORD carried in a TEXT frame. The entity will be built using the knowledge from internal 'register definitions'. Depending on this knowledge, if available, the entity type could be one of binary_sensor, sensor, text_sensor. This is done through an internal mapping between the label carrying the TEXT RECORD and the corresponding register definition.
- `hexframe` (optional - mapping): Configures behavior for HEX frames handling
  - `auto_create_entities` (optional - default: false): Same option as for `textframe`. Whenever an HEX register data is received, either broadcasted or by being queried, the component will build an entity to represent the value. Again, this entity might be a very specific one (binary_sensor, switch, sensor, number, etc) if the component has 'knowledge' through an embedded register definition or might be a plain text_sensor which will just expose the data in generic hex format (useful for debugging/reverse engineering).
  - `ping_timeout` (optional - default: 1min): The component could cyclically send PINGs to the device to keep the HEX frame layer active (see official Victron docs). To disable this feature set a timeout of `0`


Now, having configured the main component is just the first step. To make it useful by exposing data through entities is the next chapter.