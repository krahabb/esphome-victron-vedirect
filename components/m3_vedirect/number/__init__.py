import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
import esphome.const as ec

from .. import VEDirectPlatform, ve_reg

PLATFORM = VEDirectPlatform(
    "number",
    number,
    {},
    (ve_reg.CLASS.NUMERIC,),
    False,
    {
        cv.Optional(ec.CONF_MIN_VALUE): cv.float_,
        cv.Optional(ec.CONF_MAX_VALUE): cv.float_,
        cv.Optional(ec.CONF_STEP): cv.positive_float,
    },
)


async def _register_number(var, config):
    await number.register_number(
        var,
        config,
        min_value=config.get(ec.CONF_MIN_VALUE, cg.NAN),
        max_value=config.get(ec.CONF_MAX_VALUE, cg.NAN),
        step=config.get(ec.CONF_STEP, cg.NAN),
    )


PLATFORM.register_entity = _register_number

CONFIG_SCHEMA = PLATFORM.CONFIG_SCHEMA


async def to_code(config: dict):
    await PLATFORM.to_code(config)
