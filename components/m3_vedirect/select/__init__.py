from esphome import automation
import esphome.codegen as cg
from esphome.components import mqtt, select, web_server
from esphome.cpp_helpers import setup_entity

from .. import VEDirectPlatform, ve_reg

PLATFORM = VEDirectPlatform(
    "select",
    select,
    {},
    (ve_reg.CLASS.ENUM,),
    False,
)


async def _register_select(var, config):
    """
    This registration (dangerously) totally rewrites the original register_select function.
    This is needed to avoid setting the 'traits' member since it is automatically managed
    by the custom m3_vedirect::Select class.
    """
    # await select.register_select(var, config, options=[])
    cg.add(cg.App.register_select(var))
    await setup_entity(var, config)

    for conf in config.get(select.CONF_ON_VALUE, []):
        trigger = cg.new_Pvariable(conf[select.CONF_TRIGGER_ID], var)
        await automation.build_automation(
            trigger, [(cg.std_string, "x"), (cg.size_t, "i")], conf
        )

    if (mqtt_id := config.get(select.CONF_MQTT_ID)) is not None:
        mqtt_ = cg.new_Pvariable(mqtt_id, var)
        await mqtt.register_mqtt_component(mqtt_, config)

    if web_server_config := config.get(select.CONF_WEB_SERVER):
        await web_server.add_entity_config(var, web_server_config)


PLATFORM.register_entity = _register_select

CONFIG_SCHEMA = PLATFORM.CONFIG_SCHEMA


async def to_code(config: dict):
    await PLATFORM.to_code(config)
