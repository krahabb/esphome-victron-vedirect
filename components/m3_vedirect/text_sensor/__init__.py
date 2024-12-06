from esphome.components import text_sensor

from .. import VEDirectPlatform, ve_reg

# 'Special' text sensors implemented with esphome::text_sensor::TextSensor
_diagnostic_text_sensor_schema = text_sensor.text_sensor_schema(
    entity_category="diagnostic"
)

PLATFORM = VEDirectPlatform(
    "text_sensor",
    text_sensor,
    {
        "rawhexframe": VEDirectPlatform.CustomEntityDef(
            _diagnostic_text_sensor_schema, "VEDIRECT_USE_HEXFRAME"
        ),
        "rawtextframe": VEDirectPlatform.CustomEntityDef(
            _diagnostic_text_sensor_schema, "VEDIRECT_USE_TEXTFRAME"
        ),
    },
    (ve_reg.CLASS.BITMASK, ve_reg.CLASS.ENUM, ve_reg.CLASS.STRING),
    True,
)

CONFIG_SCHEMA = PLATFORM.CONFIG_SCHEMA


async def to_code(config: dict):
    await PLATFORM.to_code(config)
