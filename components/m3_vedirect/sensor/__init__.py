from esphome.components import sensor
import esphome.const as ec

from .. import VEDirectPlatform, ve_reg

PLATFORM = VEDirectPlatform(
    "sensor",
    sensor,
    {
        "run_time": VEDirectPlatform.CustomEntityDef(
            sensor.sensor_schema(
                entity_category="diagnostic",
                device_class=ec.DEVICE_CLASS_DURATION,
                unit_of_measurement=ec.UNIT_SECOND,
            ),
            "",
        ),
    },
    (ve_reg.CLASS.NUMERIC,),
    True,
)

CONFIG_SCHEMA = PLATFORM.CONFIG_SCHEMA


async def to_code(config: dict):
    await PLATFORM.to_code(config)
