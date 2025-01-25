---
title: Registers
parent: Configuration
nav_order: 1
---

[binary_sensor]: {% link configuration/entities/binary_sensor.md %}
[number]: {% link configuration/entities/number.md %}
[select]: {% link configuration/entities/select.md %}
[sensor]: {% link configuration/entities/sensor.md %}
[text_sensor]: {% link configuration/entities/text_sensor.md %}
[switch]: {% link configuration/entities/switch.md %}

## About VEDirect registers

Every data point (be it a measure, a setting, any info or so) in VEDirect is identified by a register which can be addressed (i.e. read or written) in HEX frames. Some of these registers are also broadcasted in a different format (same data) by TEXT frames where are identified by a specific label.
This component defines some properties of the registers in order to configure the entities behavior in EspHome. The first and most important property is the CLASS which defines the semantics of the data together with the DATA_TYPE which instead defines the low level encoding of data carried through HEX frames.

{: .highlight}
These informations are useful to configure the binding between a register and an entity in EspHome in order to determine their behavior. When configuring a register, you should follow the semantics as defined in the official Victron docs but you're in charge (and free) of configuring the binding. For example, even if a register is reported to behave as an `ENUM` or as a `BITMASK` in the docs, you could decide to configure that as a `NUMERIC` in a `sensor` entity so that it will render as a plain numeric value

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

## CLASS

There are 5 classes defined. Each class defines how to interpret the data and usually binds to a corresponding entity type in EspHome. For example, a `BOOLEAN` register, intuitively, should be binded to a `binary_sensor` (or `switch` if writable) entity due to its behavior. This relationship is very strong for some classes while some other might suggest multiple 'bindings' into EspHome entity platforms. We'll cover this later with some details.

- `BITMASK`: Represents a bitmask i.e. a set of bit fields each carrying a binary state. The raw data value could have any size (DATA_TYPE: UN8 - UN16 - UN32). The typical example for a BITMASK register is VE_REG_DEVICE_OFF_REASON_2 (address: 0x0207) which has a data size of 4 bytes (UN32).
- `BOOLEAN`: The register carries either 0 (false/inactive/disabled) or 1 (true/active/enabled) in a 1-byte payload (data_type: UN8).
- `ENUM`: The register can assume a value from a limited/predefined set of numeric options. It is usually encoded as uint8 (data_type: UN8). Typical example is VE_REG_DEVICE_MODE (address: 0x0200).
- `NUMERIC`: The register contains a numeric value (signed or unsigned) representing a quantity (either a measure or a setting).
- `STRING`: The value is an ANSI string of variable length (DATA_TYPE: VARIADIC).

## ACCESS

VEDirect registers can be read-only or writable and this impacts which entity platform you should use to manage the register. In general you should follow this table:

| ACCESS       | Suggested entity platform              |
| ------------ | -------------------------------------- |
| `READ_ONLY`  | [binary_sensor],[sensor],[text_sensor] |
| `READ_WRITE` | [number],[select],[switch]             |

The component uses this property internally (defined in flavors REG_DEFs) to decide which entity type to automatically instantiate when using [`auto_create_entities`]({% link configuration/index.md %}) option while there's no place to use it when manually defining entities configuration since the 'writability' is naturally implied by the entity type.
For example, you're still allowed to use a `number` entity to try writing into a `READ_ONLY` register but it'll likely end up in a frame error reply from the device.

This table summarizes the allowed bindings between a register CLASS definition and an entity platform. Details on how to configure these bindings are in the relevant platform documentation.

| Entity platform | Supported CLASSes           |
| --------------- | --------------------------- |
| [binary_sensor] | `BOOLEAN`, `BITMASK`,`ENUM` |
| [number]        | `NUMERIC`                   |
| [select]        | `ENUM`                      |
| [sensor]        | `NUMERIC`                   |
| [switch]        | `BOOLEAN`, `BITMASK`,`ENUM` |
| [text_sensor]   | `BITMASK`,`ENUM`, `STRING`  |

The following table summarizes the 'suggesteds/natural' binding relations between registers and entities:

| CLASS     | ACCESS       | Suggested entity platform                           |
| --------- | ------------ | --------------------------------------------------- |
| `BITMASK` | `READ_ONLY`  | [binary_sensor], [text_sensor]                      |
|           | `READ_WRITE` | [switch]                                            |
| `BOOLEAN` | `READ_ONLY`  | [binary_sensor]                                     |
|           | `READ_WRITE` | [switch]                                            |
| `ENUM`    | `READ_ONLY`  | [text_sensor]                                       |
|           | `READ_WRITE` | [select]                                            |
| `NUMERIC` | `READ_ONLY`  | [sensor]                                            |
|           | `READ_WRITE` | [number]                                            |
| `STRING`  | `READ_ONLY`  | [text_sensor]                                       |
|           | `READ_WRITE` | no support (likely not implemented in VEDirect too) |

## Summary

Remember the `CLASS`, beside emerging from a classification of VEDirect registers behavior according to the official docs, is just a config setting in order to describe the semantics of the underlying data to the entity platform. It doesn't mean you cannot bind what 'emerge' as an `ENUM` register (from the docs) to an EspHome `sensor` entity. For example, those previous tables state that a `sensor` can only be configured to treat the register as `NUMERIC`. That means that the value will nevertheless be interpreted as a numeric value (that's the characteristic feature of the `sensor` platform).
This will be more clear by checking each platform documentation and seeing how the `CLASS` configuration affects the treatment of raw data.
