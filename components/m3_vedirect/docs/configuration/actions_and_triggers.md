---
title: Actions and Triggers
parent: Configuration
---

## [Actions](https://esphome.io/automations/actions.html)

The component exposes two actions, both of which are dedicated to send HEX frames to the device. The difference lies in the 'level' of access:

- `m3_vedirect.send_hexframe`: is the low level version where you're able to directly encode the HEX payload to be sent. This api, will just add the checksum byte (so you don't have to provide it in the encoded string) to the full frame and send it to the device.

  ```yaml
  - m3_vedirect.send_hexframe:
      vedirect_id: "*"
      data: "1" # PING command
  ```

  It accepts 2 parameters:

  - `vedirect_id` (string): is the `id` in string form of the component. `*` will act as a wildcard matching every component in the EspHome node.
  - `data` (string): is the raw HEX payload to be sent, the final checksum byte will be automatically added (so that for the proposed example, the data sent on the channel will be `:154\n`)

- `m3_vedirect.send_command`: is the 'higher' level form suitable to easily manage HEX registers. With this `action` you can send GET/SET commands to individual registers and specify the (numeric) payload. Again the checksum and frame structure will be encoded by the component itself.

  ```yaml
  - m3_vedirect.send_command:
      vedirect_id: "vedirect_0"
      command: 8 # SET command
      register_id: 0x0200 # VE_REG_DEVICE_MODE
      data: 1 # Enable charger
      data_size: 1 # send 1 byte (register DATA_TYPE: UN8)
  ```

  Here, beside the `vedirect_id` which works as for the previous `action`, the parameters are all numeric and `data_size` specifies the number of bytes used to send the numeric payload (could be 1,2,4).

The [sample config]({% link samples/m3_vedirect_service_example.yaml %}) will show you how to configure an `HomeAssistant action` (former service) to expose these to your HA instance so that you can easily setup scripts and automations to query/config the device at the lowest possible level.

{: .highlight}

> Most of the times you'll just be done using your configured entities to interact with the device (for example by configuring a [`select`]({% link configuration/entities/select.md %}) for register `VE_REG_DEVICE_MODE`) but, nevertheless, there are some features of the VEDirect HEX interface which are not available through registers/entities and so you'd need to use this api in order to access them. For example:

- Restarting the device: This is accomplished by sending the plain command `6` (through `send_hexframe`).
- Managing Non-Volatile-Memory: Register `0xEB99` has a special behavior and could be managed by using the `send_command` action for the matter.

## [Triggers](https://esphome.io/automations/actions.html#triggers)

The component exposes the trigger `on_frame_received` for HEX frames. It can be used to access the internal struct carrying a received HEX frame. A simple example forwarding the frame payload through an `HomeAssistant event` is shown in the [example]({% link samples/m3_vedirect_service_example.yaml %})
