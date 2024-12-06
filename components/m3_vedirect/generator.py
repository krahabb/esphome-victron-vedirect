from io import TextIOWrapper
import os
import re
import typing

INDENT = "    "
GENERATED_FILENAME = "ve_reg.py"
ve_reg_py: TextIOWrapper


def _read_file(file_name: str):
    with open(file_name, encoding="utf-8") as f:
        f_data = f.read()
        # remove block comments
        f_data = re.sub(r"/\*.*\*/", "", f_data, flags=re.MULTILINE)
        # remove single line comments
        f_data = re.sub(r"//.*", "", f_data, flags=re.MULTILINE)
        # remove newlines
        f_data = re.sub(r"\n|\r", "", f_data, flags=re.MULTILINE)
        return f_data


def _extract_enum(enum_name: str, file_data: str):
    enum_match = re.search(r"enum " + enum_name + r"[^{]*{([^}]*)}", file_data)
    if enum_match:
        enum_str = enum_match.group(1)
        enum_str = re.sub(r" ", "", enum_str)
        enum_str = enum_str.strip(",")
        return enum_str.split(",")
    return []


def _declare_enum(enum_name: str, enum_list: typing.Iterable[str], enum_base: str):
    ve_reg_py.write(f"\n\nclass {enum_name}({enum_base}):\n")
    for enum_value in enum_list:
        enum_value = enum_value.split("=")[0]
        ve_reg_py.write(f"{INDENT}{enum_value} = enum.auto()\n")


def _declare_mock_enum(enum_name: str, enum_list: typing.Iterable[str], ns_mock: str):
    _declare_enum(enum_name, enum_list, "MockEnum")
    ve_reg_py.writelines(
        (
            f"\n{INDENT}@cached_property\n",
            f"{INDENT}def ns(self):\n",
            f'{INDENT}{INDENT}return {ns_mock}_struct.enum("{enum_name}", is_class=True)\n',
        )
    )


def _generate_enums(file_data: str, ns_mock: str):
    for match in re.finditer(r"enum (\w*) : [^{]*{([^}]*)}", file_data):
        enum_name = match.group(1)
        enum_items = match.group(2)
        enum_items = re.sub(r" ", "", enum_items)
        enum_items = enum_items.strip(",").split(",")
        if len(enum_items) > 1:
            _declare_mock_enum(enum_name, enum_items, ns_mock)


def generate():
    ve_reg_py.writelines(
        (
            '"""\nAUTO GENERATED - See generator.py\n"""\n\n',
            "from collections import namedtuple\n",
            "import enum\n",
            "from functools import cached_property\n",
            "\nimport esphome.codegen as cg\n",
            '\nns = cg.global_ns.namespace("m3_ve_reg")\n',
            "\n\nclass MockEnum(enum.IntEnum):\n",
            f"{INDENT}@cached_property\n",
            f"{INDENT}def ns(self):\n",
            f"{INDENT}{INDENT}pass\n",
            f"\n{INDENT}@cached_property\n",
            f"{INDENT}def enum(self):\n",
            f"{INDENT}{INDENT}return self.ns.enum(self.name)\n",
        )
    )

    ve_reg_py.writelines(('\n\nHEXFRAME_struct = ns.struct("HEXFRAME")',))
    f_ve_reg_h = _read_file("ve_reg.h")
    _generate_enums(f_ve_reg_h, "HEXFRAME")

    ve_reg_py.writelines(('\n\nREG_DEF_struct = ns.struct("REG_DEF")',))
    f_ve_reg_def_h = _read_file("ve_reg_def.h")
    _generate_enums(f_ve_reg_def_h, "REG_DEF")

    f_ve_reg_flavor_h = _read_file("ve_reg_flavor.h")
    flavors = {}
    # for match in re.finditer(r"#ifdef (\w*)", f_ve_reg_flavor_h):
    for match in re.finditer(
        r"#ifdef\s*VEDIRECT_FLAVOR_(\w*)(.*?)#endif", f_ve_reg_flavor_h
    ):
        # flavors.append(match.group(1))
        sub_flavors = []
        for sub_match in re.finditer(
            r"#define\s*VEDIRECT_FLAVOR_(\w*)", match.group(2)
        ):
            sub_flavors.append(sub_match.group(1))
        flavors[match.group(1)] = sub_flavors
    _declare_enum("Flavor", flavors, "enum.StrEnum")
    ve_reg_py.write("\n\nFLAVOR_DEPENDENCIES = {\n")
    for _flavor, _dependencies in flavors.items():
        ve_reg_py.write(f"{INDENT}Flavor.{_flavor}.name: [")
        for _dep in _dependencies:
            ve_reg_py.write(f"Flavor.{_dep}.name, ")
        ve_reg_py.write("],\n")
    ve_reg_py.write("}\n")

    f_ve_reg_register_h = _read_file("ve_reg_register.h")
    reg_defs = {}
    for match in re.finditer(r"MACRO\((.*?)\)", f_ve_reg_register_h):
        reg_def = match.group(1)
        reg_def = [s.strip() for s in reg_def.split(",")]
        reg_def_type = reg_def[3]
        reg_defs[reg_def_type] = reg_def

    _declare_mock_enum("TYPE", reg_defs, "REG_DEF")
    ve_reg_py.writelines(
        (
            '\n\nREG_DEF = namedtuple("REG_DEF", ["flavor", "cls", "register_id", "access"])\n',
            "REG_DEFS = {\n",
        )
    )
    for reg_def_type, reg_def in reg_defs.items():
        ve_reg_py.write(
            f'{INDENT}TYPE.{reg_def_type}.name: REG_DEF("{reg_def[0]}", CLASS.{reg_def[1].split("_")[0]}, {reg_def[2]}, ACCESS.{reg_def[4]}),\n'
        )

    ve_reg_py.write("}\n")


if __name__ == "__main__":
    # check if we're running in vscode root working dir
    if os.path.exists("esphome"):
        current_dir = os.getcwd()
        vedirect_path = os.path.join(
            current_dir, "esphome", "components", "m3_vedirect"
        )
        os.chdir(vedirect_path)

    with open(GENERATED_FILENAME, mode="w", encoding="utf-8") as ve_reg_py:
        generate()
