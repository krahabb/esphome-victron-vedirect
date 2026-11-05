from io import TextIOWrapper
import os
from pathlib import Path
import re
import typing

if typing.TYPE_CHECKING:
    pass

INDENT_PY = "    "
VE_REG_PY = "ve_reg.py"
ve_reg_py: TextIOWrapper


def _read_file(file_name: str):
    with Path(file_name).open(encoding="utf-8") as f:
        f_data = f.read()
        # remove block comments
        f_data = re.sub(r"/\*.*?\*/", "", f_data, flags=re.DOTALL)
        # remove single line comments
        f_data = re.sub(r"//.*", "", f_data)
        # remove newlines
        return re.sub(r"\n|\r", "", f_data)


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
        ve_reg_py.write(f"{INDENT_PY}{enum_value} = enum.auto()\n")


def _declare_mock_enum(enum_name: str, enum_list: typing.Iterable[str], ns_mock: str):
    _declare_enum(enum_name, enum_list, "MockEnum")
    ve_reg_py.writelines(
        (
            f"\n{INDENT_PY}@cached_property\n",
            f"{INDENT_PY}def ns(self):\n",
            f'{INDENT_PY}{INDENT_PY}return {ns_mock}_struct.enum("{enum_name}", is_class=True)\n',
        )
    )


def _declare_enums(file_data: str, ns_mock: str):
    ve_reg_py.writelines((f'\n\n{ns_mock}_struct = ns.struct("{ns_mock}")\n',))
    for match in re.finditer(r"enum (\w*) : [^{]*{([^}]*)}", file_data):
        enum_name = match.group(1)
        enum_items = match.group(2)
        enum_items = re.sub(r" ", "", enum_items)
        enum_items = enum_items.strip(",").split(",")
        if len(enum_items) > 1:
            _declare_mock_enum(enum_name, enum_items, ns_mock)


def generate_docs():
    # dirty trick to fool vscode linting
    if typing.TYPE_CHECKING:
        from . import ve_reg
    else:
        # this should actually import the currently generated file
        import ve_reg

    import enum

    def _generate_TABLE(name: str, items: list[tuple[str]]):
        column_count = len(items[0])
        column_sizes = [0] * column_count
        for row in items:
            for column_index, column in enumerate(row):
                column_sizes[column_index] = max(
                    column_sizes[column_index], len(column)
                )

        table = [
            f"<!--BEGIN {name}_TABLE-->\n\n",
            f"| {' | '.join(column.ljust(column_sizes[column_index]) for column_index, column in enumerate(items[0]))} |\n",
            f"| {' | '.join('-' * column_size for column_size in column_sizes)} |\n",
        ]

        table.extend(
            [
                f"| {' | '.join(column.ljust(column_sizes[column_index]) for column_index, column in enumerate(row))} |\n"
                for row in items[1:]
            ]
        )

        table.append(f"\n<!--END {name}_TABLE-->")
        return table

    def _generate_REG_DEF_TABLE():
        return _generate_TABLE(
            "REG_DEF",
            [
                ("register", "hex address", "class", "r/w", "flavor"),
                *[
                    (
                        reg_def_type,
                        f"0x{reg_def.register_id:04X}",
                        reg_def.cls.name,
                        reg_def.access.name,
                        reg_def.flavor,
                    )
                    for reg_def_type, reg_def in ve_reg.REG_DEFS.items()
                    if not reg_def_type.startswith("UNKNOWN_")
                ],
            ],
        )

    def _generate_FLAVOR_TABLE():
        return _generate_TABLE(
            "FLAVOR",
            [
                ("flavor", "inherited flavors"),
                *[
                    (flavor, ", ".join(flavors))
                    for flavor, flavors in ve_reg.FLAVOR_DEPENDENCIES.items()
                ],
            ],
        )

    def generate_tables():
        """
        Fill the docs files with table data generated from code
        """
        reg_def_table = _generate_REG_DEF_TABLE()
        flavor_table = _generate_FLAVOR_TABLE()

        for doc_filename in (
            "docs/README.md",
            "docs/configuration/REG_DEFS.md",
        ):
            with Path(doc_filename).open(encoding="utf-8") as doc_file:
                doc_src = doc_file.read()

            doc_dst = re.sub(
                r"<!--BEGIN REG_DEF_TABLE-->.*<!--END REG_DEF_TABLE-->",
                "".join(reg_def_table),
                doc_src,
                flags=re.DOTALL,
            )

            doc_dst = re.sub(
                r"<!--BEGIN FLAVOR_TABLE-->.*<!--END FLAVOR_TABLE-->",
                "".join(flavor_table),
                doc_dst,
                flags=re.DOTALL,
            )

            if doc_dst != doc_src:
                with Path(doc_filename).open(
                    mode="w", encoding="utf-8", newline="\n"
                ) as doc_file:
                    doc_file.write(doc_dst)

    def generate_sample_config(flavor: str):
        FLAVOR_TEMPLATE = "docs/samples/m3_vedirect_flavor_template_.yaml"
        FLAVOR_EXAMPLE = "docs/samples/m3_vedirect_flavor_%s_example.yaml"
        INDENT_YAML = "  "

        with Path(FLAVOR_TEMPLATE).open(encoding="utf-8") as flavor_template_file:
            flavor_example = flavor_template_file.read()

        flavor_example = flavor_example.replace("# flavor", f"flavor: [{flavor}]")

        class Platform(enum.Enum):
            binary_sensor = "binary_sensor"
            number = "number"
            select = "select"
            sensor = "sensor"
            switch = "switch"
            text_sensor = "text_sensor"

        entities = {
            Platform.binary_sensor: {},
            Platform.number: {},
            Platform.select: {},
            Platform.sensor: {},
            Platform.switch: {},
            Platform.text_sensor: {},
        }

        # map 'class' 'access' to an entity type to be built
        entity_map = {
            ve_reg.ACCESS.CONSTANT: {
                ve_reg.CLASS.BITMASK: entities[Platform.text_sensor],
                ve_reg.CLASS.BOOLEAN: entities[Platform.binary_sensor],
                ve_reg.CLASS.ENUM: entities[Platform.text_sensor],
                ve_reg.CLASS.NUMERIC: entities[Platform.sensor],
                ve_reg.CLASS.STRING: entities[Platform.text_sensor],
                ve_reg.CLASS.VOID: entities[Platform.text_sensor],
            },
            ve_reg.ACCESS.READ_ONLY: {
                ve_reg.CLASS.BITMASK: entities[Platform.text_sensor],
                ve_reg.CLASS.BOOLEAN: entities[Platform.binary_sensor],
                ve_reg.CLASS.ENUM: entities[Platform.text_sensor],
                ve_reg.CLASS.NUMERIC: entities[Platform.sensor],
                ve_reg.CLASS.STRING: entities[Platform.text_sensor],
                ve_reg.CLASS.VOID: entities[Platform.text_sensor],
            },
            ve_reg.ACCESS.READ_WRITE: {
                ve_reg.CLASS.BITMASK: entities[Platform.text_sensor],
                ve_reg.CLASS.BOOLEAN: entities[Platform.switch],
                ve_reg.CLASS.ENUM: entities[Platform.select],
                ve_reg.CLASS.NUMERIC: entities[Platform.number],
                ve_reg.CLASS.STRING: entities[Platform.text_sensor],
                ve_reg.CLASS.VOID: entities[Platform.text_sensor],
            },
        }

        def _inflate_flavor(flavor, inflated: set):
            """Returns a set with all of the sub-flavors that would be automatically defined
            when defining the provided 'flavor' (see ve_reg_flavor.h)"""
            for sub_flavor in ve_reg.FLAVOR_DEPENDENCIES[flavor]:
                inflated.add(sub_flavor)
                _inflate_flavor(sub_flavor, inflated)
            return inflated

        flavors = _inflate_flavor(flavor, set())
        flavors.add(flavor)

        for reg_def_type, reg_def in ve_reg.REG_DEFS.items():
            # TODO: check the flavor..right now use ALL
            if reg_def.flavor in flavors:
                entity_map[reg_def.access][reg_def.cls][reg_def_type] = reg_def

        for platform, platform_entities in entities.items():
            if platform_entities:
                flavor_example += "".join(
                    (
                        f"\n{platform.name}:\n",
                        f"{INDENT_YAML}- platform: m3_vedirect\n",
                        f"{INDENT_YAML}{INDENT_YAML}vedirect_id: vedirect_0\n",
                        f"{INDENT_YAML}{INDENT_YAML}vedirect_entities:\n",
                    )
                )
                for reg_def_type in platform_entities:
                    flavor_example += "".join(
                        (
                            f"{INDENT_YAML}{INDENT_YAML}{INDENT_YAML}- register: {reg_def_type}\n",
                            f'{INDENT_YAML}{INDENT_YAML}{INDENT_YAML}{INDENT_YAML}name: "{reg_def_type}"\n',
                        )
                    )

        with Path(FLAVOR_EXAMPLE % flavor).open(
            mode="w", encoding="utf-8", newline="\n"
        ) as flavor_example_file:
            flavor_example_file.write(flavor_example)

    generate_tables()

    for flavor in ("ALL", "MPPT", "INV", "BMV"):
        generate_sample_config(flavor)


def generate():
    ve_reg_py.writelines(
        (
            '"""\nAUTO GENERATED - See generator.py\n"""\n\n',
            "import enum\n",
            "from functools import cached_property\n",
            "import typing\n",
            "\nimport esphome.codegen as cg\n",
            '\nns = cg.esphome_ns.namespace("m3_vedirect").namespace("m3_ve_reg")\n',
            "\n\nclass MockEnum(enum.IntEnum):\n",
            f"{INDENT_PY}@cached_property\n",
            f"{INDENT_PY}def ns(self):\n",
            f"{INDENT_PY}{INDENT_PY}pass\n",
            f"\n{INDENT_PY}@cached_property\n",
            f"{INDENT_PY}def enum(self):\n",
            f"{INDENT_PY}{INDENT_PY}return self.ns.enum(self.name)\n",
        )
    )

    f_ve_reg_h = _read_file("ve_reg.h")
    _declare_enums(f_ve_reg_h, "HEXFRAME")

    f_ve_reg_def_h = _read_file("ve_reg_def.h")
    _declare_enums(f_ve_reg_def_h, "REG_DEF")

    f_ve_reg_flavor_h = _read_file("ve_reg_flavor.h")
    flavors = {}
    # for match in re.finditer(r"#ifdef (\w*)", f_ve_reg_flavor_h):
    for match in re.finditer(
        r"#ifdef\s*VEDIRECT_FLAVOR_(\w*)(.*?)#endif", f_ve_reg_flavor_h
    ):
        flavors[match.group(1)] = [
            sub_match.group(1)
            for sub_match in re.finditer(
                r"#define\s*VEDIRECT_FLAVOR_(\w*)", match.group(2)
            )
        ]
    _declare_enum("Flavor", flavors, "enum.StrEnum")
    ve_reg_py.write("\n\nFLAVOR_DEPENDENCIES = {\n")
    for _flavor, _dependencies in flavors.items():
        if _dependencies:
            ve_reg_py.write(f"{INDENT_PY}Flavor.{_flavor}.name: [\n")
            ve_reg_py.writelines(
                f"{INDENT_PY}{INDENT_PY}Flavor.{_dep}.name,\n" for _dep in _dependencies
            )
            ve_reg_py.write(f"{INDENT_PY}],\n")
        else:
            ve_reg_py.write(f"{INDENT_PY}Flavor.{_flavor}.name: [],\n")
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
            '\n\nREG_DEF = typing.NamedTuple("REG_DEF", [("flavor", str), ("cls", CLASS), ("register_id", int), ("access", ACCESS)])\n',
            "REG_DEFS = {\n",
        )
    )
    for reg_def_type, reg_def in reg_defs.items():
        ve_reg_py.write(
            f'{INDENT_PY}TYPE.{reg_def_type}.name: REG_DEF("{reg_def[0]}", CLASS.{reg_def[1].split("_")[0]}, {reg_def[2]}, ACCESS.{reg_def[4]}),\n'
        )

    ve_reg_py.write("}\n")


if __name__ == "__main__":
    # check if we're running in vscode root working dir
    if Path("esphome").exists():
        os.chdir(Path.cwd() / "esphome" / "components" / "m3_vedirect")

    with Path(VE_REG_PY).open(mode="w", encoding="utf-8", newline="\n") as ve_reg_py:
        generate()

    # Here we'll use the generated ve_reg.py to build some docs
    generate_docs()
