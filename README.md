# esphome-victron-vedirect

This [EspHome external component](https://esphome.io/components/external_components) contains a complete implementation to support [Victron VEDirect](https://www.victronenergy.com/upload/documents/VE.Direct-Protocol-3.33.pdf) serial interface. It has parsers for both the HEX and the TEXT protocol data carried over the communication channel and allows to also issue commands in order to configure the Victron device.

In order to use an ESP (both legacy ESP8266 and/or any ESP32 variant) to connect to a VEDirect hardware interface it uses one of the UART buses available in the ESP platform. The code itself is optimized (or can be) to use the minimum amount of resources but, depending on the number of entities configured you might be better off using one of the more capable ESP32 versions so to have more RAM and CPU power for the task.

The details for hardware interfacing can be found online and this repo doesn't provide many clues but you have to basically adapt and connect the VEDirect RX/TX data lines to the ESP uart bus. Now this might be as simple as directly wiring those RX/TX lines to the UART pins of the ESP if the VEDirect device uses 3.3 V operation or, if the interface carries 5V signals, you might be better off using an optocoupler (H11L1 might be needed because of the data transfer rate used in the signaling) or any form of signal level adaptation.

Before using this component I strongly suggest to understand how VEDirect works by deeply checking these (and any other possibly available) docs:
- https://www.victronenergy.com/upload/documents/VE.Direct-Protocol-3.33.pdf
- https://www.victronenergy.com/upload/documents/BMV-7xx-HEX-Protocol.pdf
- https://www.victronenergy.com/upload/documents/BlueSolar-HEX-protocol.pdf
- https://www.victronenergy.com/upload/documents/VE.Direct-HEX-Protocol-Phoenix-Inverter.pdf

The first document being the overall technical description and explaining what is achievable with the TEXT protocol while the others are a more detailed description of the HEX protocol features available among the different classes of devices.

Sadly enough, beside those few documents, Victron doesn't expose a very detailed 'implementation manual' for every device/family, especially the new ones (like the RS families of devices for instance) so, many times you'll have to infer or guess how to interface with these by looking at their most close counterparts for which documentation is available. As an example, I'm using this code and the knowledge from those docs to succesfully (at least partially) interface a Multi RS inverter which contains both MPPTs and INVERTERs features (plus many which are very specific and for which there's no official doc available).
At any rate, for basic sensor readings and configuration those documents are enough since, even among different devices and classes, the configuration registers are pretty almost the same.

## How to use
In order to include the component in your EspHome project use the [instructions](https://esphome.io/components/external_components). Something in the lines:
```yaml
external_components:
  source: github://krahabb/esphome-victron-vedirect
```
There are some sample configurations with a basic explanation of the features for different scenarios:
- [m3_vedirect_basic_example.yaml](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/m3_vedirect_basic_example.yaml)
- [m3_vedirect_minimal_example.yaml](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/m3_vedirect_minimal_example.yaml)
- [m3_vedirect_service_example.yaml](https://github.com/krahabb/esphome-victron-vedirect/blob/main/components/m3_vedirect/m3_vedirect_service_example.yaml)

More help and wiki will come.

Good interfacing!
