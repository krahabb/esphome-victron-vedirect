from esphome.components import sensor

from .. import VEDirectPlatform, ve_reg

PLATFORM = VEDirectPlatform(
    "sensor",
    sensor,
    {},
    (ve_reg.CLASS.NUMERIC,),
    True,
)

CONFIG_SCHEMA = PLATFORM.CONFIG_SCHEMA


async def to_code(config: dict):
    await PLATFORM.to_code(config)
