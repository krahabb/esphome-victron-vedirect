"""
AUTO GENERATED - See generator.py
"""

from collections import namedtuple
import enum
from functools import cached_property

import esphome.codegen as cg

ns = cg.global_ns.namespace("m3_ve_reg")


class MockEnum(enum.IntEnum):
    @cached_property
    def ns(self):
        pass

    @cached_property
    def enum(self):
        return self.ns.enum(self.name)


HEXFRAME_struct = ns.struct("HEXFRAME")


class COMMAND(MockEnum):
    Ping = enum.auto()
    Done = enum.auto()
    AppVersion = enum.auto()
    Unknown = enum.auto()
    ProductId = enum.auto()
    Error = enum.auto()
    PingResp = enum.auto()
    Restart = enum.auto()
    Get = enum.auto()
    Set = enum.auto()
    Async = enum.auto()

    @cached_property
    def ns(self):
        return HEXFRAME_struct.enum("COMMAND", is_class=True)


class DATA_TYPE(MockEnum):
    VARIADIC = enum.auto()
    UN8 = enum.auto()
    UN16 = enum.auto()
    UN32 = enum.auto()
    SN8 = enum.auto()
    SN16 = enum.auto()
    SN32 = enum.auto()
    _COUNT = enum.auto()

    @cached_property
    def ns(self):
        return HEXFRAME_struct.enum("DATA_TYPE", is_class=True)


REG_DEF_struct = ns.struct("REG_DEF")


class CLASS(MockEnum):
    VOID = enum.auto()
    BITMASK = enum.auto()
    BOOLEAN = enum.auto()
    ENUM = enum.auto()
    NUMERIC = enum.auto()
    STRING = enum.auto()

    @cached_property
    def ns(self):
        return REG_DEF_struct.enum("CLASS", is_class=True)


class ACCESS(MockEnum):
    CONSTANT = enum.auto()
    READ_ONLY = enum.auto()
    READ_WRITE = enum.auto()

    @cached_property
    def ns(self):
        return REG_DEF_struct.enum("ACCESS", is_class=True)


class UNIT(MockEnum):
    NONE = enum.auto()
    UNKNOWN = enum.auto()
    A = enum.auto()
    V = enum.auto()
    VA = enum.auto()
    W = enum.auto()
    Ah = enum.auto()
    kWh = enum.auto()
    SOC_PERCENTAGE = enum.auto()
    minute = enum.auto()
    CELSIUS = enum.auto()
    KELVIN = enum.auto()
    UNIT_COUNT = enum.auto()

    @cached_property
    def ns(self):
        return REG_DEF_struct.enum("UNIT", is_class=True)


class SCALE(MockEnum):
    S_1 = enum.auto()
    S_0_1 = enum.auto()
    S_0_01 = enum.auto()
    S_0_001 = enum.auto()
    S_0_25 = enum.auto()
    SCALE_COUNT = enum.auto()

    @cached_property
    def ns(self):
        return REG_DEF_struct.enum("SCALE", is_class=True)


class Flavor(enum.StrEnum):
    ALL = enum.auto()
    MULTI_RS = enum.auto()
    INV_PHNX = enum.auto()
    CHG_PHNX = enum.auto()
    MPPT_BS = enum.auto()
    MPPT_RS = enum.auto()
    BMV60 = enum.auto()
    BMV70 = enum.auto()
    BMV71 = enum.auto()
    MPPT = enum.auto()
    BMV = enum.auto()
    CHG = enum.auto()
    INV = enum.auto()
    ANY = enum.auto()


FLAVOR_DEPENDENCIES = {
    Flavor.ALL.name: [
        Flavor.MULTI_RS.name,
        Flavor.MPPT_BS.name,
        Flavor.MPPT_RS.name,
        Flavor.INV_PHNX.name,
        Flavor.CHG_PHNX.name,
        Flavor.BMV60.name,
        Flavor.BMV70.name,
        Flavor.BMV71.name,
    ],
    Flavor.MULTI_RS.name: [
        Flavor.INV.name,
        Flavor.MPPT_RS.name,
    ],
    Flavor.INV_PHNX.name: [
        Flavor.INV.name,
    ],
    Flavor.CHG_PHNX.name: [
        Flavor.CHG.name,
    ],
    Flavor.MPPT_BS.name: [
        Flavor.MPPT.name,
    ],
    Flavor.MPPT_RS.name: [
        Flavor.MPPT.name,
    ],
    Flavor.BMV60.name: [
        Flavor.BMV.name,
    ],
    Flavor.BMV70.name: [
        Flavor.BMV.name,
    ],
    Flavor.BMV71.name: [
        Flavor.BMV.name,
    ],
    Flavor.MPPT.name: [
        Flavor.CHG.name,
    ],
    Flavor.BMV.name: [
        Flavor.ANY.name,
    ],
    Flavor.CHG.name: [
        Flavor.ANY.name,
    ],
    Flavor.INV.name: [
        Flavor.ANY.name,
    ],
    Flavor.ANY.name: [],
}


class TYPE(MockEnum):
    BLE_MODE = enum.auto()
    PRODUCT_ID = enum.auto()
    APP_VER = enum.auto()
    SERIAL_NUMBER = enum.auto()
    MODEL_NAME = enum.auto()
    CAPABILITIES = enum.auto()
    CAPABILITIES_BLE = enum.auto()
    DEVICE_MODE = enum.auto()
    DEVICE_STATE = enum.auto()
    DEVICE_OFF_REASON = enum.auto()
    DEVICE_OFF_REASON_2 = enum.auto()
    INVERTER_DEVICE_STATE = enum.auto()
    CHARGER_DEVICE_STATE = enum.auto()
    AC_OUT_VOLTAGE_SETPOINT = enum.auto()
    MPPT_TRACKERS = enum.auto()
    UNKNOWN_0305 = enum.auto()
    UNKNOWN_0310 = enum.auto()
    UNKNOWN_0311 = enum.auto()
    WARNING_REASON = enum.auto()
    ALARM_REASON = enum.auto()
    ALARM_LOW_VOLTAGE_SET = enum.auto()
    ALARM_LOW_VOLTAGE_CLEAR = enum.auto()
    RELAY_CONTROL = enum.auto()
    RELAY_MODE = enum.auto()
    UNKNOWN_0FFC = enum.auto()
    TTG = enum.auto()
    SOC = enum.auto()
    SOLAR_ACTIVITY = enum.auto()
    TIME_OF_DAY = enum.auto()
    AC_OUT_VOLTAGE = enum.auto()
    AC_OUT_CURRENT = enum.auto()
    AC_OUT_APPARENT_POWER = enum.auto()
    SHUTDOWN_LOW_VOLTAGE_SET = enum.auto()
    VOLTAGE_RANGE_MIN = enum.auto()
    VOLTAGE_RANGE_MAX = enum.auto()
    U_AC_OUT_VOLTAGE = enum.auto()
    U_AC_OUT_CURRENT = enum.auto()
    U_AC_OUT_APPARENT_POWER = enum.auto()
    UNKNOWN_2216 = enum.auto()
    UNKNOWN_2250 = enum.auto()
    UNKNOWN_2251 = enum.auto()
    U_AC_OUT_CURRENT_MA = enum.auto()
    UNKNOWN_D5C8 = enum.auto()
    UNKNOWN_D5CA = enum.auto()
    UNKNOWN_D5CB = enum.auto()
    MPPT_TRACKER_MODE_1 = enum.auto()
    PANEL_VOLTAGE_1 = enum.auto()
    PANEL_POWER_1 = enum.auto()
    PANEL_CURRENT_1 = enum.auto()
    MPPT_TRACKER_MODE_2 = enum.auto()
    PANEL_VOLTAGE_2 = enum.auto()
    PANEL_POWER_2 = enum.auto()
    PANEL_CURRENT_2 = enum.auto()
    MPPT_TRACKER_MODE_3 = enum.auto()
    PANEL_VOLTAGE_3 = enum.auto()
    PANEL_POWER_3 = enum.auto()
    PANEL_CURRENT_3 = enum.auto()
    MPPT_TRACKER_MODE_4 = enum.auto()
    PANEL_VOLTAGE_4 = enum.auto()
    PANEL_POWER_4 = enum.auto()
    PANEL_CURRENT_4 = enum.auto()
    BATTERY_RIPPLE_VOLTAGE = enum.auto()
    DC_CHANNEL1_VOLTAGE = enum.auto()
    DC_CHANNEL1_POWER = enum.auto()
    DC_CHANNEL1_CURRENT = enum.auto()
    LOAD_OUTPUT_STATE = enum.auto()
    LOAD_CURRENT = enum.auto()
    MPPT_TRACKER_MODE = enum.auto()
    PANEL_MAXIMUM_VOLTAGE = enum.auto()
    PANEL_VOLTAGE = enum.auto()
    PANEL_POWER = enum.auto()
    PANEL_CURRENT = enum.auto()
    PANEL_MAXIMUM_CURRENT = enum.auto()
    MAXIMUM_POWER_YESTERDAY = enum.auto()
    YIELD_YESTERDAY = enum.auto()
    MAXIMUM_POWER_TODAY = enum.auto()
    YIELD_TODAY = enum.auto()
    CHARGER_VOLTAGE = enum.auto()
    CHARGER_CURRENT = enum.auto()
    CHR_ERROR_CODE = enum.auto()
    CHR_INTERNAL_TEMPERATURE = enum.auto()
    USER_YIELD = enum.auto()
    SYSTEM_YIELD = enum.auto()
    BAT_TEMPERATURE = enum.auto()
    DC_MONITOR_MODE = enum.auto()
    ALARM_BUZZER = enum.auto()

    @cached_property
    def ns(self):
        return REG_DEF_struct.enum("TYPE", is_class=True)


REG_DEF = namedtuple("REG_DEF", ["flavor", "cls", "register_id", "access"])
REG_DEFS = {
    TYPE.BLE_MODE.name: REG_DEF("ANY", CLASS.BITMASK, 0x0090, ACCESS.READ_WRITE),
    TYPE.PRODUCT_ID.name: REG_DEF("ANY", CLASS.VOID, 0x0100, ACCESS.CONSTANT),
    TYPE.APP_VER.name: REG_DEF("ANY", CLASS.VOID, 0x0102, ACCESS.CONSTANT),
    TYPE.SERIAL_NUMBER.name: REG_DEF("ANY", CLASS.STRING, 0x010A, ACCESS.CONSTANT),
    TYPE.MODEL_NAME.name: REG_DEF("ANY", CLASS.STRING, 0x010B, ACCESS.CONSTANT),
    TYPE.CAPABILITIES.name: REG_DEF("ANY", CLASS.BITMASK, 0x0140, ACCESS.CONSTANT),
    TYPE.CAPABILITIES_BLE.name: REG_DEF("ANY", CLASS.BITMASK, 0x0150, ACCESS.CONSTANT),
    TYPE.DEVICE_MODE.name: REG_DEF("ANY", CLASS.ENUM, 0x0200, ACCESS.READ_WRITE),
    TYPE.DEVICE_STATE.name: REG_DEF("ANY", CLASS.ENUM, 0x0201, ACCESS.READ_ONLY),
    TYPE.DEVICE_OFF_REASON.name: REG_DEF(
        "ANY", CLASS.BITMASK, 0x0205, ACCESS.READ_ONLY
    ),
    TYPE.DEVICE_OFF_REASON_2.name: REG_DEF(
        "ANY", CLASS.BITMASK, 0x0207, ACCESS.READ_ONLY
    ),
    TYPE.INVERTER_DEVICE_STATE.name: REG_DEF(
        "INV", CLASS.ENUM, 0x0209, ACCESS.READ_ONLY
    ),
    TYPE.CHARGER_DEVICE_STATE.name: REG_DEF(
        "CHG", CLASS.ENUM, 0x020A, ACCESS.READ_ONLY
    ),
    TYPE.AC_OUT_VOLTAGE_SETPOINT.name: REG_DEF(
        "INV", CLASS.NUMERIC, 0x0230, ACCESS.READ_WRITE
    ),
    TYPE.MPPT_TRACKERS.name: REG_DEF("MPPT_RS", CLASS.NUMERIC, 0x0244, ACCESS.CONSTANT),
    TYPE.UNKNOWN_0305.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0x0305, ACCESS.READ_ONLY
    ),
    TYPE.UNKNOWN_0310.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0x0310, ACCESS.READ_ONLY
    ),
    TYPE.UNKNOWN_0311.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0x0311, ACCESS.READ_ONLY
    ),
    TYPE.WARNING_REASON.name: REG_DEF("ANY", CLASS.BITMASK, 0x031C, ACCESS.READ_ONLY),
    TYPE.ALARM_REASON.name: REG_DEF("ANY", CLASS.BITMASK, 0x031E, ACCESS.READ_ONLY),
    TYPE.ALARM_LOW_VOLTAGE_SET.name: REG_DEF(
        "INV", CLASS.NUMERIC, 0x0320, ACCESS.READ_WRITE
    ),
    TYPE.ALARM_LOW_VOLTAGE_CLEAR.name: REG_DEF(
        "INV", CLASS.NUMERIC, 0x0321, ACCESS.READ_WRITE
    ),
    TYPE.RELAY_CONTROL.name: REG_DEF("ANY", CLASS.BOOLEAN, 0x034E, ACCESS.READ_WRITE),
    TYPE.RELAY_MODE.name: REG_DEF("ANY", CLASS.ENUM, 0x034F, ACCESS.READ_WRITE),
    TYPE.UNKNOWN_0FFC.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0x0FFC, ACCESS.READ_ONLY
    ),
    TYPE.TTG.name: REG_DEF("BMV", CLASS.NUMERIC, 0x0FFE, ACCESS.READ_ONLY),
    TYPE.SOC.name: REG_DEF("BMV", CLASS.NUMERIC, 0x0FFF, ACCESS.READ_ONLY),
    TYPE.SOLAR_ACTIVITY.name: REG_DEF("MPPT", CLASS.BOOLEAN, 0x2030, ACCESS.READ_ONLY),
    TYPE.TIME_OF_DAY.name: REG_DEF("MPPT", CLASS.NUMERIC, 0x2031, ACCESS.READ_ONLY),
    TYPE.AC_OUT_VOLTAGE.name: REG_DEF("INV", CLASS.NUMERIC, 0x2200, ACCESS.READ_ONLY),
    TYPE.AC_OUT_CURRENT.name: REG_DEF("INV", CLASS.NUMERIC, 0x2201, ACCESS.READ_ONLY),
    TYPE.AC_OUT_APPARENT_POWER.name: REG_DEF(
        "INV", CLASS.NUMERIC, 0x2205, ACCESS.READ_ONLY
    ),
    TYPE.SHUTDOWN_LOW_VOLTAGE_SET.name: REG_DEF(
        "INV", CLASS.NUMERIC, 0x2210, ACCESS.READ_WRITE
    ),
    TYPE.VOLTAGE_RANGE_MIN.name: REG_DEF("INV", CLASS.NUMERIC, 0x2211, ACCESS.CONSTANT),
    TYPE.VOLTAGE_RANGE_MAX.name: REG_DEF("INV", CLASS.NUMERIC, 0x2212, ACCESS.CONSTANT),
    TYPE.U_AC_OUT_VOLTAGE.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0x2213, ACCESS.READ_ONLY
    ),
    TYPE.U_AC_OUT_CURRENT.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0x2214, ACCESS.READ_ONLY
    ),
    TYPE.U_AC_OUT_APPARENT_POWER.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0x2215, ACCESS.READ_ONLY
    ),
    TYPE.UNKNOWN_2216.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0x2216, ACCESS.READ_ONLY
    ),
    TYPE.UNKNOWN_2250.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0x2250, ACCESS.READ_ONLY
    ),
    TYPE.UNKNOWN_2251.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0x2251, ACCESS.READ_ONLY
    ),
    TYPE.U_AC_OUT_CURRENT_MA.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0xD3A1, ACCESS.READ_ONLY
    ),
    TYPE.UNKNOWN_D5C8.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0xD5C8, ACCESS.READ_ONLY
    ),
    TYPE.UNKNOWN_D5CA.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0xD5CA, ACCESS.READ_ONLY
    ),
    TYPE.UNKNOWN_D5CB.name: REG_DEF(
        "MULTI_RS", CLASS.NUMERIC, 0xD5CB, ACCESS.READ_ONLY
    ),
    TYPE.MPPT_TRACKER_MODE_1.name: REG_DEF(
        "MPPT_RS", CLASS.ENUM, 0xECC3, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_VOLTAGE_1.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECCB, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_POWER_1.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECCC, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_CURRENT_1.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECCD, ACCESS.READ_ONLY
    ),
    TYPE.MPPT_TRACKER_MODE_2.name: REG_DEF(
        "MPPT_RS", CLASS.ENUM, 0xECD3, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_VOLTAGE_2.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECDB, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_POWER_2.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECDC, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_CURRENT_2.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECDD, ACCESS.READ_ONLY
    ),
    TYPE.MPPT_TRACKER_MODE_3.name: REG_DEF(
        "MPPT_RS", CLASS.ENUM, 0xECE3, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_VOLTAGE_3.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECEB, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_POWER_3.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECEC, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_CURRENT_3.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECED, ACCESS.READ_ONLY
    ),
    TYPE.MPPT_TRACKER_MODE_4.name: REG_DEF(
        "MPPT_RS", CLASS.ENUM, 0xECF3, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_VOLTAGE_4.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECFB, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_POWER_4.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECFC, ACCESS.READ_ONLY
    ),
    TYPE.PANEL_CURRENT_4.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xECFD, ACCESS.READ_ONLY
    ),
    TYPE.BATTERY_RIPPLE_VOLTAGE.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xED8B, ACCESS.READ_ONLY
    ),
    TYPE.DC_CHANNEL1_VOLTAGE.name: REG_DEF(
        "ANY", CLASS.NUMERIC, 0xED8D, ACCESS.READ_ONLY
    ),
    TYPE.DC_CHANNEL1_POWER.name: REG_DEF(
        "BMV", CLASS.NUMERIC, 0xED8E, ACCESS.READ_ONLY
    ),
    TYPE.DC_CHANNEL1_CURRENT.name: REG_DEF(
        "ANY", CLASS.NUMERIC, 0xED8F, ACCESS.READ_ONLY
    ),
    TYPE.LOAD_OUTPUT_STATE.name: REG_DEF(
        "MPPT", CLASS.BOOLEAN, 0xEDA8, ACCESS.READ_ONLY
    ),
    TYPE.LOAD_CURRENT.name: REG_DEF("MPPT", CLASS.NUMERIC, 0xEDAD, ACCESS.READ_ONLY),
    TYPE.MPPT_TRACKER_MODE.name: REG_DEF("MPPT", CLASS.ENUM, 0xEDB3, ACCESS.READ_ONLY),
    TYPE.PANEL_MAXIMUM_VOLTAGE.name: REG_DEF(
        "MPPT", CLASS.NUMERIC, 0xEDB8, ACCESS.CONSTANT
    ),
    TYPE.PANEL_VOLTAGE.name: REG_DEF("MPPT", CLASS.NUMERIC, 0xEDBB, ACCESS.READ_ONLY),
    TYPE.PANEL_POWER.name: REG_DEF("MPPT", CLASS.NUMERIC, 0xEDBC, ACCESS.READ_ONLY),
    TYPE.PANEL_CURRENT.name: REG_DEF("MPPT", CLASS.NUMERIC, 0xEDBD, ACCESS.READ_ONLY),
    TYPE.PANEL_MAXIMUM_CURRENT.name: REG_DEF(
        "MPPT_RS", CLASS.NUMERIC, 0xEDBF, ACCESS.CONSTANT
    ),
    TYPE.MAXIMUM_POWER_YESTERDAY.name: REG_DEF(
        "MPPT", CLASS.NUMERIC, 0xEDD0, ACCESS.READ_ONLY
    ),
    TYPE.YIELD_YESTERDAY.name: REG_DEF("MPPT", CLASS.NUMERIC, 0xEDD1, ACCESS.READ_ONLY),
    TYPE.MAXIMUM_POWER_TODAY.name: REG_DEF(
        "MPPT", CLASS.NUMERIC, 0xEDD2, ACCESS.READ_ONLY
    ),
    TYPE.YIELD_TODAY.name: REG_DEF("MPPT", CLASS.NUMERIC, 0xEDD3, ACCESS.READ_ONLY),
    TYPE.CHARGER_VOLTAGE.name: REG_DEF("CHG", CLASS.NUMERIC, 0xEDD5, ACCESS.READ_ONLY),
    TYPE.CHARGER_CURRENT.name: REG_DEF("CHG", CLASS.NUMERIC, 0xEDD7, ACCESS.READ_ONLY),
    TYPE.CHR_ERROR_CODE.name: REG_DEF("CHG", CLASS.ENUM, 0xEDDA, ACCESS.READ_ONLY),
    TYPE.CHR_INTERNAL_TEMPERATURE.name: REG_DEF(
        "CHG", CLASS.NUMERIC, 0xEDDB, ACCESS.READ_ONLY
    ),
    TYPE.USER_YIELD.name: REG_DEF("MPPT", CLASS.NUMERIC, 0xEDDC, ACCESS.READ_ONLY),
    TYPE.SYSTEM_YIELD.name: REG_DEF("MPPT", CLASS.NUMERIC, 0xEDDD, ACCESS.READ_ONLY),
    TYPE.BAT_TEMPERATURE.name: REG_DEF("ANY", CLASS.NUMERIC, 0xEDEC, ACCESS.READ_ONLY),
    TYPE.DC_MONITOR_MODE.name: REG_DEF(
        "BMV71", CLASS.NUMERIC, 0xEEB8, ACCESS.READ_ONLY
    ),
    TYPE.ALARM_BUZZER.name: REG_DEF("BMV", CLASS.BOOLEAN, 0xEEFC, ACCESS.READ_WRITE),
}
