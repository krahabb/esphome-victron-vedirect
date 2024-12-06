from esphome.components import binary_sensor

from .. import VEDIRECT_BINARY_ENTITY_BASE_SCHEMA, VEDirectPlatform, ve_reg

# Manager special sensors
_diagnostic_binary_sensor_schema = binary_sensor.binary_sensor_schema(
    entity_category="diagnostic"
)


PLATFORM = VEDirectPlatform(
    "binary_sensor",
    binary_sensor,
    {
        "link_connected": VEDirectPlatform.CustomEntityDef(
            _diagnostic_binary_sensor_schema, ""
        ),
    },
    (ve_reg.CLASS.BOOLEAN, ve_reg.CLASS.BITMASK, ve_reg.CLASS.ENUM),
    True,
    VEDIRECT_BINARY_ENTITY_BASE_SCHEMA,
)

CONFIG_SCHEMA = PLATFORM.CONFIG_SCHEMA


async def to_code(config: dict):
    await PLATFORM.to_code(config)
