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
        cv.Required(ec.CONF_MIN_VALUE): cv.float_,
        cv.Required(ec.CONF_MAX_VALUE): cv.float_,
        cv.Required(ec.CONF_STEP): cv.positive_float,
    },
)


async def _register_number(var, config):
    await number.register_number(
        var,
        config,
        min_value=config[ec.CONF_MIN_VALUE],
        max_value=config[ec.CONF_MAX_VALUE],
        step=config[ec.CONF_STEP],
    )


PLATFORM.register_entity = _register_number

CONFIG_SCHEMA = PLATFORM.CONFIG_SCHEMA


async def to_code(config: dict):
    await PLATFORM.to_code(config)
