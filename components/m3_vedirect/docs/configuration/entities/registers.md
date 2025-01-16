---
title: Registers
parent: Entities
nav_order: 1
---

## About VEDirect registers

Every data point (be it a measure, a setting, any info or so) in VEDirect is identified by a register which can be addressed (i.e. read or written) in HEX frames. Some of these registers are also broadcasted in a different format (same data) by TEXT frames where are identified by a specific label.
This component defines some properties of the registers in order to configure the entities behavior in EspHome. The first and most important property is the CLASS which defines the semantics of the data together with the DATA_TYPE which instead defines the low level encoding of data carried through HEX frames.

{: .highlight}
These informations are useful to configure the mapping between a register and an entity in EspHome in order to determine their behavior. When configuring a register, you should follow the semantics as defined in the official Victron docs but you're in charge (and free) of configuring the mapping. For example, even if a register is reported to behave as an ENUM or as a BITMASK in the docs, you could decide to configure that as a NUMERIC one so that it will render as a plain numeric value (to be used as a raw `sensor` for example)

## CLASS

There are 5 classes defined. Each class defines how to interpret the data and usually maps to a corresponding entity type in EspHome. For example, a `BOOLEAN` register, intuitively, should be binded to a `binary_sensor` (or `switch` if writable) entity due its behavior. This relationship is very strong for some classes while some other might suggest multiple 'mappings' into EspHome entity platforms. We'll cover this later with some details.

- `BITMASK`: Represents a bitmask i.e. a set of bit fields each carrying a binary state. The raw data value could have any size (DATA_TYPE: UN8 - UN16 - UN32). The typical example for a BITMASK register is VE_REG_DEVICE_OFF_REASON_2 (address: 0x0207) which has a data size of 4 bytes (UN32).
- `BOOLEAN`: The register carries either 0 (false/inactive/disabled) or 1 (true/active/enabled) in a 1-byte payload (data_type: UN8).
- `ENUM`: The register can assume a value from a limited/predefined set of numeric options. It is usually encoded as uint8 (data_type: UN8). Typical example is VE_REG_DEVICE_MODE (address: 0x0200)
- `NUMERIC`: The register contains a numeric value (signed or unsigned) representing a quantity (either a measure or a setting)
- `STRING`: The value is an ANSI string

This table summarizes the allowed bindings between a register CLASS definition and an entity platforms. Details on how to configure these bindings are in the relevant platform documentation>

| CLASS     | Supported entity platform                                                    |
| --------- | ---------------------------------------------------------------------------- |
| `BITMASK` | [binary_sensor](binary_sensor), [switch](switch), [text_sensor](text_sensor) |
| `BOOLEAN` | [binary_sensor](binary_sensor), [switch](switch)                             |
| `ENUM`    | [binary_sensor](binary_sensor), [select](select), [text_sensor](text_sensor) |
| `NUMERIC` | [number](number), [sensor](sensor)                                           |
| `STRING`  | [text_sensor](text_sensor)                                                   |

## DATA_TYPE

This defines the low level encoding of data carried in the HEX frame when querying/setting the register. Every register has its own 'DATA_TYPE' whichever the CLASS. This property is only meaningful when configuring binding for HEX frames since TEXT frames carry data in a string which is always 'smartly' parsed by the component.
We have:

| DATA_TYPE | data size (bytes) | signed/unsigned | raw level encoding (c++)                                                                    |
| --------- | ----------------- | --------------- | ------------------------------------------------------------------------------------------- |
| VARIADIC  | unknown           |                 | The data length is variable or not well defined - typically used for STRING class registers |
| UN8       | 1                 | unsigned        | uint8                                                                                       |
| UN16      | 2                 | unsigned        | uint16                                                                                      |
| UN32      | 4                 | unsigned        | uint32                                                                                      |
| SN8       | 1                 | signed          | int8                                                                                        |
| SN16      | 2                 | signed          | int16                                                                                       |
| SN32      | 4                 | signed          | int32                                                                                       |
