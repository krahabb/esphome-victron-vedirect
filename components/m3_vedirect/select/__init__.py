from esphome.components import select

from .. import VEDirectPlatform, ve_reg

PLATFORM = VEDirectPlatform(
    "select",
    select,
    {},
    (ve_reg.CLASS.ENUM,),
    False,
)


async def _register_select(var, config):
    await select.register_select(var, config, options=[])


PLATFORM.register_entity = _register_select

CONFIG_SCHEMA = PLATFORM.CONFIG_SCHEMA


async def to_code(config: dict):
    await PLATFORM.to_code(config)
