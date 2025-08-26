from esphome.components import select

from .. import VEDirectPlatform, ve_reg

PLATFORM = VEDirectPlatform(
    "select",
    select,
    {},
    (ve_reg.CLASS.ENUM,),
    False,
)

CONFIG_SCHEMA = PLATFORM.CONFIG_SCHEMA


async def to_code(config: dict):
    await PLATFORM.to_code(config)
