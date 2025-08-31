from collections import namedtuple
import enum
from functools import partial
import typing

from esphome import automation
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
import esphome.const as ec
from esphome.core import CORE
import esphome.cpp_generator as cpp

from . import ve_reg

CODEOWNERS = ["@krahabb"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True

ESPHOME_VERSION = cv.Version.parse(ec.__version__)

ENUM_DEF_struct = ve_reg.ns.struct("ENUM_DEF")
ENUM_DEF_LOOKUP_DEF_struct = ENUM_DEF_struct.struct("LOOKUP_DEF")


m3_vedirect_ns = cg.esphome_ns.namespace("m3_vedirect")
Manager = m3_vedirect_ns.class_("Manager", uart.UARTDevice, cg.Component)
HexFrame = m3_vedirect_ns.class_("HexFrame")
HexFrame_const_ref = HexFrame.operator("const").operator("ref")

HexFrameTrigger = Manager.class_(
    "HexFrameTrigger", automation.Trigger.template(HexFrame_const_ref)
)

CONF_VEDIRECT_ID = "vedirect_id"
CONF_VEDIRECT_ENTITIES = "vedirect_entities"
CONF_TEXTFRAME = "textframe"
CONF_HEXFRAME = "hexframe"


# common schema for entities:
def validate_register_id():
    return cv.hex_int_range(min=0, max=65535)


def validate_mock_enum(enum_class: type[ve_reg.MockEnum]):
    return cv.enum({_enum.name: _enum.enum for _enum in enum_class})


def validate_str_enum(enum_class: type[enum.StrEnum]):
    return cv.enum({_enum.name: _enum for _enum in enum_class})


def validate_numeric_scale():
    """Allows 'scale' to be set either as a typed enum from REG_DEF::SCALE or a float value.
    float value must be one of the normalized."""

    def _convert_to_float(scale: ve_reg.SCALE):
        return float(scale.name[2:].replace("_", "."))

    return cv.enum(
        {
            _convert_to_float(scale): scale.enum
            for scale in ve_reg.SCALE
            if scale != ve_reg.SCALE.SCALE_COUNT
        }
        | {
            scale.name: scale.enum
            for scale in ve_reg.SCALE
            if scale != ve_reg.SCALE.SCALE_COUNT
        }
    )


def _validate_enum_lookup_def(value):
    """Validate the configuration is a single item dict with {value}: {label}.
    representing an ENUM_DEF::LOOKUP_DEF item.
    """
    if isinstance(value, dict) and len(value) == 1:
        for enum_value, enum_label in value.items():
            return cv.positive_int(enum_value), cv.string_strict(enum_label)

    raise cv.Invalid(
        f"Expected a single item dict with {{value}}: {{'label'}}, got {value}"
    )


ENUM_LOOKUP_DEF_LIST_SCHEMA = cv.Schema([_validate_enum_lookup_def])


def validate_enum_lookup_def_list(value):
    """Validate this configuration option to be an ordered list of ENUM_DEF::LOOKUP_DEF.

    If the config value is not a list, it is automatically converted to a
    single-item list.

    None and empty dictionaries are converted to empty lists.

    Code inspired by cv.ensure_list
    """

    cv.check_not_templatable(value)
    if value is None or (isinstance(value, dict) and not value):
        return []
    if not isinstance(value, list):
        return [_validate_enum_lookup_def(value)]
    result = ENUM_LOOKUP_DEF_LIST_SCHEMA(value)
    enum_value = -1
    for lookup_def in result:
        if lookup_def[0] <= enum_value:
            raise cv.Invalid(f"Expected a list in ascending order, got {result}")
        enum_value = lookup_def[0]

    return result


CONF_REGISTER = "register"
CONF_TEXT_LABEL = "text_label"
# (HEX) Register schema
CONF_REG_DEF_ID = "reg_def_id"
CONF_ENUM_DEF_ID = "enum_def_id"
CONF_ADDRESS = "address"
CONF_DATA_TYPE = "data_type"
VEDIRECT_REGISTER_SCHEMA = {
    cv.GenerateID(CONF_REG_DEF_ID): cv.declare_id(ve_reg.REG_DEF_struct),
    cv.GenerateID(CONF_ENUM_DEF_ID): cv.declare_id(ENUM_DEF_struct),
    # binds to the corresponding HEX register
    cv.Optional(CONF_ADDRESS, default=0): validate_register_id(),
    # configures the format of the HEX register
    cv.Optional(
        CONF_DATA_TYPE, default=ve_reg.DATA_TYPE.VARIADIC.name
    ): validate_mock_enum(ve_reg.DATA_TYPE),
}
CONF_TEXT_SCALE = "text_scale"
CONF_SCALE = "scale"
CONF_UNIT = "unit"
VEDIRECT_REGISTER_CLASS_SCHEMAS = {
    ve_reg.CLASS.BOOLEAN: cv.Schema({}),
    ve_reg.CLASS.BITMASK: validate_enum_lookup_def_list,
    ve_reg.CLASS.ENUM: validate_enum_lookup_def_list,
    ve_reg.CLASS.NUMERIC: cv.Schema(
        {
            cv.Optional(CONF_SCALE, default=1): validate_numeric_scale(),
            cv.Optional(CONF_TEXT_SCALE): validate_numeric_scale(),
            cv.Optional(CONF_UNIT, default=ve_reg.UNIT.NONE): validate_mock_enum(
                ve_reg.UNIT
            ),
        }
    ),
    ve_reg.CLASS.STRING: cv.Schema({}),
}
# TEXT record schema
VEDIRECT_TEXTRECORD_SCHEMA = {
    # binds to the corresponding TEXT frame field
    cv.Optional(CONF_TEXT_LABEL): cv.string,
}


# Basic schema for binary-like entities: binary_sensor, switch
CONF_MASK = "mask"
VEDIRECT_BINARY_ENTITY_BASE_SCHEMA = {
    cv.Optional(CONF_MASK): cv.uint32_t,
}


def global_object_construct(id_: cpp.ID, *args):
    obj = cpp.MockObj(id_, ".")
    cg.add_global(cpp.RawStatement(f"{id_.type} {id_}({cpp.ExpressionList(*args)});"))
    return obj


def local_assignment(lvalue: cpp.MockObj, rvalue: cpp.MockObj):
    cg.add(cpp.RawStatement(f"{lvalue} = {cpp.safe_exp(rvalue)};"))


def define_symbol(symbol: str):
    cg.add_build_flag(f"-D{symbol}=")
    # Even if not strictly necessary since our (library) code doesn't import esphome\defines.h
    # so that our build environment just relies on cli -D option forwarded to the compiler,
    # this is useful to inspect what's the config environment by just inspecting the built
    # esphome/defines.h
    cg.add_define(symbol)


def define_use_hexframe():
    define_symbol("VEDIRECT_USE_HEXFRAME")


def define_use_textframe():
    define_symbol("VEDIRECT_USE_TEXTFRAME")


def inflate_flavor(flavor, inflated: set):
    """Returns a set with all of the sub-flavors that would be automatically defined
    when defining the provided 'flavor' (see ve_reg_flavor.h)"""
    sub_flavors = ve_reg.FLAVOR_DEPENDENCIES[flavor]
    for sub_flavor in sub_flavors:
        inflated.add(sub_flavor)
        inflate_flavor(sub_flavor, inflated)
    return inflated


def inflate_flavors(flavors: typing.Iterable):
    """
    Given a collection of flavors, expand this with all the 'sub' flavors defined by each of the
    original set.
    """
    inflated = set(flavors)
    for flavor in flavors:
        inflate_flavor(flavor, inflated)

    return inflated


def deflate_flavors(flavors: typing.Iterable):
    """
    Processes the configuration CONF_FLAVOR and extracts a 'minimal' flavors list.
    Since users could type any flavor in the configuration, some of these might be automatically
    added by our ve_reg_flavor.h macro processing ('group flavors' or 'base flavors').
    When we add the build flag these redundant definitions will raise some warnings and we want
    to avoid that and only set the minimal appropriate set of flavors.
    """
    deflated_flavors = set(flavors)
    for flavor in flavors:
        inflated = inflate_flavor(flavor, set())
        duplicates = deflated_flavors & inflated
        for sub_flavor in duplicates:
            deflated_flavors.remove(sub_flavor)

    return deflated_flavors


class VEDirectPlatform:
    COMPONENT_NS: typing.Final = m3_vedirect_ns

    RegisterEntityT = typing.Callable[[typing.Any, typing.Any], typing.Awaitable[None]]
    register_entity: RegisterEntityT

    CustomEntityDef = namedtuple("CustomEntityDef", ("schema", "define"))

    def __init__(
        self,
        snake_name: str,
        base_module,
        custom_entities: dict[
            str, CustomEntityDef
        ],  # additional custom entities definitions beside vedirect ones
        vedirect_classes: typing.Iterable[
            ve_reg.CLASS
        ],  # vedirect classes supported by this platform
        vedirect_has_text: bool,  # indicates if this platform supports entities for the TEXT frames
        *vedirect_schemas,  # extra schemas added to base vedirect_schema
        register_entity: RegisterEntityT
        | None = None,  # optional custom register_entity function
    ):
        self.snake_name = snake_name
        self.class_name = "".join([p.capitalize() for p in snake_name.split("_")])
        self.base_module = base_module
        self.entity_class = m3_vedirect_ns.class_(
            self.class_name, getattr(base_module, self.class_name)
        )
        # grab some symbols from the base platform
        self.register_entity = register_entity or getattr(
            base_module, f"register_{snake_name}"
        )
        self.new_base_entity: typing.Callable = getattr(
            base_module, f"new_{snake_name}"
        )
        self.vedirect_classes = vedirect_classes
        self.custom_entities = custom_entities

        register_schema = dict(VEDIRECT_REGISTER_SCHEMA)
        for _cls in self.vedirect_classes:
            register_schema |= {
                cv.Exclusive(
                    _cls.name.lower(), "class"
                ): VEDIRECT_REGISTER_CLASS_SCHEMAS[_cls]
            }
        if vedirect_has_text:
            register_schema |= VEDIRECT_TEXTRECORD_SCHEMA

        base_schema: cv.Schema = getattr(base_module, f"{snake_name}_schema")(
            self.entity_class
        )
        self.vedirect_schema = base_schema.extend(
            {
                cv.Required(CONF_REGISTER): cv.Any(
                    validate_mock_enum(ve_reg.TYPE), cv.Schema(register_schema)
                ),
            },
            *vedirect_schemas,
        )

    @property
    def CONFIG_SCHEMA(self):
        return cv.Schema(
            {
                cv.Required(CONF_VEDIRECT_ID): cv.use_id(Manager),
                cv.Optional(CONF_VEDIRECT_ENTITIES): cv.ensure_list(
                    self.vedirect_schema
                ),
            }
            | {
                cv.Optional(type): custom_entity_def.schema
                for type, custom_entity_def in self.custom_entities.items()
            }
        ).add_extra(self._validate_platform)

    def _validate_platform(self, config):
        # Ensure CONF_TYPE is available based off Manager flavor
        if CONF_VEDIRECT_ENTITIES in config:
            flavors = MANAGERS_CONFIG[config[CONF_VEDIRECT_ID]][CONF_FLAVOR]
            inflated_flavors = inflate_flavors(flavors)
            for vedirect_entity_config in config[CONF_VEDIRECT_ENTITIES]:
                register = vedirect_entity_config[CONF_REGISTER]
                if isinstance(register, str):
                    type_flavor = ve_reg.REG_DEFS[register].flavor
                    if type_flavor not in inflated_flavors:
                        raise cv.Invalid(
                            f"Entity type {register} is defined for flavor {type_flavor} which is not available in configured flavors:{flavors}"
                        )
        return config

    async def new_vedirect_entity(self, config, manager):
        entity = cg.new_Pvariable(config[ec.CONF_ID], manager)

        register_config = config[CONF_REGISTER]
        if isinstance(register_config, str):
            cg.add(manager.init_register(entity, register_config))
        elif isinstance(register_config, dict):
            register_id = register_config[CONF_ADDRESS]
            if register_id:
                define_use_hexframe()
            for _cls in ve_reg.CLASS:
                _cls_key = _cls.name.lower()
                if _cls_key in register_config:
                    class_config = register_config[_cls_key]
                    match _cls:
                        case ve_reg.CLASS.BITMASK | ve_reg.CLASS.ENUM:
                            enum_def = global_object_construct(
                                register_config[CONF_ENUM_DEF_ID],
                                class_config,
                            )
                            reg_def = global_object_construct(
                                register_config[CONF_REG_DEF_ID],
                                register_id,
                                register_config[CONF_DATA_TYPE],
                                _cls.enum,
                                cpp.UnaryOpExpression("&", enum_def),
                            )

                        case ve_reg.CLASS.NUMERIC:
                            scale = class_config[CONF_SCALE]
                            reg_def = global_object_construct(
                                register_config[CONF_REG_DEF_ID],
                                register_id,
                                register_config[CONF_DATA_TYPE],
                                class_config[CONF_UNIT],
                                scale,
                                class_config.get(CONF_TEXT_SCALE, scale),
                            )
                        case _:
                            reg_def = global_object_construct(
                                register_config[CONF_REG_DEF_ID],
                                register_id,
                                register_config[CONF_DATA_TYPE],
                                _cls.enum,
                            )
                    break
            else:
                reg_def = global_object_construct(
                    register_config[CONF_REG_DEF_ID],
                    register_id,
                    register_config[CONF_DATA_TYPE],
                    ve_reg.CLASS.VOID.enum,
                )

            cg.add(manager.init_register(entity, cpp.UnaryOpExpression("&", reg_def)))

            if CONF_TEXT_LABEL in register_config:
                define_use_textframe()
                cg.add(manager.init_register(entity, register_config[CONF_TEXT_LABEL]))

        # configure binary-like entities
        if CONF_MASK in config:
            cg.add(entity.set_mask(config[CONF_MASK]))

        return entity

    async def to_code(self, config: dict):
        manager = await cg.get_variable(config[CONF_VEDIRECT_ID])
        cg.add(
            cpp.RawStatement(
                f"m3_vedirect::Register::register_platform(m3_vedirect::Register::{self.class_name}, m3_vedirect::{self.class_name}::build_entity);"
            )
        )
        for entity_key, entity_config in config.items():
            if entity_key == CONF_VEDIRECT_ENTITIES:
                for _entity_config in entity_config:
                    entity = await self.new_vedirect_entity(_entity_config, manager)
                    await self.register_entity(entity, _entity_config)
                continue
            if entity_key in self.custom_entities:
                define = self.custom_entities[entity_key].define
                if define:
                    for _define in define.split(","):
                        define_symbol(_define)

                entity = await self.new_base_entity(entity_config)
                cg.add(getattr(manager, f"set_{entity_key}")(entity))

        if cv.Version(2025, 8, 0) <= ESPHOME_VERSION:
            # Platforms entities vectors are now statically pre-allocated so we have
            # to tell EspHome core how many entities we want. This is especially needed for dynamically
            # allocated ones since we don't know the number at config time ;)
            _dynamic_entities_count = (
                1  # ensure at least 1 slot whatever the config for little safety
            )
            # Config for dynamic entities is set at the manager component level.
            # We provide default values when 'auto_create_entities' is set with a
            # bool value (legacy validator)
            for _key, _default_entities_count in {
                CONF_HEXFRAME: 100,
                CONF_TEXTFRAME: 32,
            }.items():
                try:
                    _count_or_bool = MANAGERS_CONFIG[config[CONF_VEDIRECT_ID]][_key][
                        CONF_AUTO_CREATE_ENTITIES
                    ]
                    _dynamic_entities_count += (
                        _default_entities_count
                        if _count_or_bool is True
                        else int(_count_or_bool)
                    )
                except KeyError:
                    pass
            # BEWARE: we should loop on this api but we take the direct path
            # CORE.register_platform_component(self.snake_name, None)
            CORE.platform_counts[self.snake_name] += _dynamic_entities_count


# main component (Manager) schema
MANAGERS_CONFIG = {}


def validate_manager(config):
    # Caching the manager(s) config since we'll need that to validate
    # platforms configuration. Especially the 'flavor' setting might affect
    # some other validators.
    MANAGERS_CONFIG[config[ec.CONF_ID]] = config
    return config


CONF_AUTO_CREATE_ENTITIES = "auto_create_entities"
CONF_PING_TIMEOUT = "ping_timeout"
CONF_FLAVOR = "flavor"
CONF_ON_FRAME_RECEIVED = "on_frame_received"
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Manager),
            cv.Optional(ec.CONF_NAME): cv.string_strict,
            cv.Optional(
                CONF_FLAVOR, default=[flavor.name for flavor in ve_reg.Flavor]
            ): cv.ensure_list(validate_str_enum(ve_reg.Flavor)),
            cv.Optional(CONF_TEXTFRAME): cv.Schema(
                {
                    cv.Optional(CONF_AUTO_CREATE_ENTITIES): cv.Any(
                        cv.boolean, cv.positive_int
                    ),
                }
            ),
            cv.Optional(CONF_HEXFRAME): cv.Schema(
                {
                    cv.Optional(CONF_AUTO_CREATE_ENTITIES): cv.Any(
                        cv.boolean, cv.positive_int
                    ),
                    cv.Optional(CONF_PING_TIMEOUT): cv.positive_time_period_seconds,
                    cv.Optional(CONF_ON_FRAME_RECEIVED): automation.validate_automation(
                        {
                            cv.GenerateID(ec.CONF_TRIGGER_ID): cv.declare_id(
                                HexFrameTrigger
                            ),
                        }
                    ),
                }
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
    .add_extra(validate_manager)
)


async def to_code(config: dict):
    """
    Our code has several conditional defines used to optimize (strip/optimze)
    code generation for size and performance depending on usage.
    For example, the symbols VEDIRECT_USE_HEXFRAME, VEDIRECT_USE_TEXTFRAME
    enable structures and parsers for their respective frame format so that
    by defining only one of them, the code could be stripped off of the
    non-relevant feature. Our config validation/generation will then
    automatically define any of these pre-processor symbols when that feature
    is requested by configuration so that there's no need for the user to think about
    explicitly enabling it through a dedicated config. This is also enforced
    when setting up automations.
    We're using 'add_build_flag' instead of 'add_define' because not every
    compilation unit is actually including the 'esphome/defines.h'
    """
    var = cg.new_Pvariable(config[ec.CONF_ID])
    cg.add(var.set_vedirect_id(str(var.base)))
    cg.add(var.set_vedirect_name(config.get(ec.CONF_NAME, str(var.base))))
    for flavor in deflate_flavors(config[CONF_FLAVOR]):
        cg.add_build_flag(f"-DVEDIRECT_FLAVOR_{flavor}")
    if CONF_TEXTFRAME in config:
        define_use_textframe()
        config_textframe = config[CONF_TEXTFRAME]
        if CONF_AUTO_CREATE_ENTITIES in config_textframe:
            cg.add(
                var.set_auto_create_text_entities(
                    config_textframe[CONF_AUTO_CREATE_ENTITIES]
                )
            )
    if CONF_HEXFRAME in config:
        define_use_hexframe()
        config_hexframe = config[CONF_HEXFRAME]
        if CONF_AUTO_CREATE_ENTITIES in config_hexframe:
            cg.add(
                var.set_auto_create_hex_entities(
                    config_hexframe[CONF_AUTO_CREATE_ENTITIES]
                )
            )
        if CONF_PING_TIMEOUT in config_hexframe:
            cg.add(var.set_ping_timeout(config_hexframe[CONF_PING_TIMEOUT]))

        for conf in config_hexframe.get(CONF_ON_FRAME_RECEIVED, []):
            trigger = cg.new_Pvariable(conf[ec.CONF_TRIGGER_ID], var)
            await automation.build_automation(
                trigger, [(HexFrame_const_ref, "hexframe")], conf
            )

    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)


# ACTIONS

_CTYPE_VALIDATOR_MAP = {
    cv.string: cg.std_string,
    cv.int_: cg.int_,
    validate_register_id: cg.uint16,
}


async def action_to_code(
    schema_def: dict[cv.Optional, object], config, action_id, template_args, args
):
    # all of our currently defined actions are based on HEX frame support
    define_use_hexframe()
    var = cg.new_Pvariable(action_id, template_args)
    for _schema_key, _ctype in schema_def.items():
        _key_name = _schema_key.schema
        if _key_name in config:
            template_ = await cg.templatable(
                config[_key_name], args, _CTYPE_VALIDATOR_MAP[_ctype]
            )
            cg.add(getattr(var, f"set_{_key_name}")(template_))
    return var


CONF_COMMAND = "command"
CONF_REGISTER_ID = "register_id"
CONF_DATA_SIZE = "data_size"
MANAGER_ACTIONS = {
    "send_hexframe": {
        cv.Optional(CONF_VEDIRECT_ID, default=""): cv.string,
        cv.Required(ec.CONF_DATA): cv.string,
    },
    "send_command": {
        cv.Optional(CONF_VEDIRECT_ID, default=""): cv.string,
        cv.Required(CONF_COMMAND): cv.int_,
        cv.Optional(CONF_REGISTER_ID): validate_register_id,
        cv.Optional(ec.CONF_DATA): cv.int_,
        cv.Optional(CONF_DATA_SIZE): cv.int_,
    },
}

for _action_name, _schema_def in MANAGER_ACTIONS.items():
    _action = Manager.class_(f"Action_{_action_name}", automation.Action)
    _schema = cv.Schema(
        {
            _schema_key: cv.templatable(_ctype)
            for _schema_key, _ctype in _schema_def.items()
        }
    )
    automation.register_action(f"m3_vedirect.{_action_name}", _action, _schema)(
        partial(action_to_code, _schema_def)
    )
