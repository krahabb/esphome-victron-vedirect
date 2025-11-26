from esphome.components import switch
import esphome.config_validation as cv

from .. import VEDIRECT_BINARY_ENTITY_BASE_SCHEMA, VEDirectPlatform, ve_reg

PLATFORM = VEDirectPlatform(
    "switch",
    switch,
    {},
    (ve_reg.CLASS.BOOLEAN, ve_reg.CLASS.BITMASK, ve_reg.CLASS.ENUM),
    False,
    VEDIRECT_BINARY_ENTITY_BASE_SCHEMA,
    {
        cv.Optional(switch.CONF_RESTORE_MODE, default="DISABLED"): cv.enum(
            {
                "DISABLED": switch.SwitchRestoreMode.SWITCH_RESTORE_DISABLED,
            },
            upper=True,
            space="_",
        ),
    },
)

CONFIG_SCHEMA = PLATFORM.CONFIG_SCHEMA


async def to_code(config: dict):
    await PLATFORM.to_code(config)
