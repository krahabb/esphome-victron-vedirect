#pragma once
#include <vector>
#include <string>
#include <cstring>
#include "ve_reg.h"
#include "ve_reg_enum.h"
#include "ve_reg_register.h"
#include "ve_reg_text.h"

namespace m3_ve_reg {

// clang-format off

// clang-format on

#pragma pack(push, 1)

/// @brief Base class acting as 'enum helper' for registers containing enumerated values
struct ENUM_DEF {
 public:
  // allow an easy parameterization of the underlying enum representation
  // we'll start with a believe of always being uint8 but we might need to templatize this struct
  // should some enum registers hold bigger data representations
  typedef uint8_t enum_t;
  static constexpr enum_t VALUE_UNKNOWN = 0xFF;

  struct LOOKUP_DEF {
    enum_t value;
    const char *label;
    bool operator<(const enum_t &value) const { return this->value < value; }
  };

  struct LOOKUP_RESULT {
    int index;
    LOOKUP_DEF *lookup_def;
    bool added;
  };

  typedef const char *(*lookup_func_t)(enum_t value);

  std::vector<LOOKUP_DEF> LOOKUPS;
  ENUM_DEF(std::initializer_list<LOOKUP_DEF> initializer_list) : LOOKUPS(initializer_list) {}

  /// @brief Lookups the label associated with value in current definitions
  /// @param value
  /// @return nullptr if no label definition for value
  const char *lookup_label(enum_t value);
  /// @brief Lookups a matching label in current definitions
  /// @param label
  /// @return nullptr if no lookup definition
  const LOOKUP_DEF *lookup_value(const char *label);

  /// @brief Lookups (eventually adding) the label associated with value in current definitions
  /// @param value
  /// @return the whole lookup definition with additional context in LOOKUP_RESULT
  LOOKUP_RESULT get_lookup(enum_t value);
};

/// @brief Helper for registers carrying BITMASK class data. This is implemented mainly as an enumeration
/// but will add helpers for treating the enums so defined as bitmask values
struct BITMASK_DEF : public ENUM_DEF {
 public:
  typedef uint32_t bitmask_t;
  static constexpr bitmask_t VALUE_UNKNOWN = 0xFFFFFFFF;
  BITMASK_DEF(std::initializer_list<LOOKUP_DEF> initializer_list) : ENUM_DEF(initializer_list) {}
};

// declare the enum helpers structs for BITMASK/ENUM registers
#define _DEF_ENUM_VOID N
#define _DEF_ENUM_BOOLEAN N
#define _DEF_ENUM_BITMASK Y
#define _DEF_ENUM_BITMASK_S N
#define _DEF_ENUM_ENUM Y
#define _DEF_ENUM_ENUM_S N
#define _DEF_ENUM_NUMERIC N
#define _DEF_ENUM_STRING N
#define _ENUMS_ITEM(enum, value) enum = value
#define _DECLARE_ENUMS(cls, register_id, label, ...) \
  struct VE_REG_##label##_##cls : public cls##_DEF { \
   public: \
    enum : enum_t { cls##_##label(_ENUMS_ITEM) }; \
  }; \
  extern cls##_DEF VE_REG_##label##_##cls##_DEF;
#define DECLARE_ENUMS(flavor, cls, register_id, label, ...) \
  IF(DEF_##flavor)(IF(_DEF_ENUM_##cls)(_DECLARE_ENUMS(cls, register_id, label, ...)))

REGISTERS_COMMON(DECLARE_ENUMS)

struct REG_DEF {
#define DECLARE_REG_LABEL(flavor, cls, register_id, label, ...) IF(DEF_##flavor)(label, )
  enum TYPE : uint16_t { REGISTERS_COMMON(DECLARE_REG_LABEL) TYPE_COUNT };
#undef DECLARE_REG_LABEL

  /// @brief Defines the data semantics of this register
  enum CLASS : uint8_t {
    VOID,     // untyped data. Generally rendered by the 'default' handler (HEX for TextSensors)
    BITMASK,  // represents a set of bit flags
    BOOLEAN,  // boolean state represented by 0 -> false, 1 -> true
    ENUM,     // enumeration data
    NUMERIC,  // numeric data (either signed or unsigned)
    STRING,   // (ascii?) content
  };

  enum ACCESS : uint8_t {
    CONSTANT = 0,    // fixed read-only value
    READ_ONLY = 1,   // measure
    READ_WRITE = 2,  // configuration
  };

  typedef HEXFRAME::DATA_TYPE DATA_TYPE;

  // configuration symbols for numeric sensors
  enum UNIT : uint8_t {
    NONE,     // no unit - no state class
    UNKNOWN,  // unknown unit - state class: measurement
    A,
    V,
    VA,
    W,
    Ah,
    kWh,
    SOC_PERCENTAGE,
    minute,
    CELSIUS,
    KELVIN,
    UNIT_COUNT,
  };
  static const char *UNITS[UNIT::UNIT_COUNT];

  enum SCALE : uint8_t {
    S_1,
    S_0_1,
    S_0_01,
    S_0_001,
    S_0_25,
    SCALE_COUNT,
  };
  static const float SCALE_TO_SCALE[SCALE::SCALE_COUNT];

  static constexpr register_id_t REGISTER_UNDEFINED = 0x0000;

  const register_id_t register_id;
  const char *const label;  // not relevant for manually built (in config.yaml) REG_DEF(s)
  CLASS cls : 3;
  ACCESS access : 2;
  DATA_TYPE data_type : 3;  // only relevant for BITMASK and NUMERIC (ENUM are UN8 though)
  union {
    struct {
      ENUM_DEF *enum_def;
    };
    struct {
      UNIT unit : 4;
      SCALE scale : 4;
      SCALE text_scale : 4;
    };
  };

  static const REG_DEF DEFS[TYPE::TYPE_COUNT];
  bool operator<(const register_id_t register_id) const { return this->register_id < register_id; }
  static const REG_DEF *find_register_id(register_id_t register_id);
  static const REG_DEF *find_type(TYPE type) { return (type < ARRAY_COUNT(DEFS)) ? DEFS + type : nullptr; }

  // These constructors are general purpose and can be used in any context. Specifically they're used mostly in
  // EspHome code generation

  /// @brief Acting as a versatile default constructor
  REG_DEF(register_id_t register_id = REGISTER_UNDEFINED, DATA_TYPE data_type = DATA_TYPE::VARIADIC,
          CLASS cls = CLASS::VOID, ENUM_DEF *enum_def = nullptr)
      : register_id(register_id),
        label(nullptr),
        cls(cls),
        access(ACCESS::CONSTANT),
        data_type(data_type),
        enum_def(enum_def) {}

  /// @brief Constructor for NUMERIC register definitions
  REG_DEF(register_id_t register_id, DATA_TYPE data_type, UNIT unit, SCALE scale, SCALE text_scale)
      : register_id(register_id),
        label(nullptr),
        cls(CLASS::NUMERIC),
        access(ACCESS::CONSTANT),
        data_type(data_type),
        unit(unit),
        scale(scale),
        text_scale(text_scale) {}

  // These other constructors are suited for usage in REGISTERS_COMMON(DEFINE_REG_DEF)

  /// @brief Constructor for VOID, BOOLEAN, or STRING register definitions
  REG_DEF(register_id_t register_id, const char *label, CLASS cls, ACCESS access)
      : register_id(register_id),
        label(label),
        cls(cls),
        access(access),
        data_type(cls == CLASS::BOOLEAN ? DATA_TYPE::UN8 : DATA_TYPE::VARIADIC),
        enum_def(nullptr) {}
  /// @brief Constructor for BITMASK registers definitions
  REG_DEF(register_id_t register_id, const char *label, ACCESS access, DATA_TYPE data_type, ENUM_DEF *enum_def)
      : register_id(register_id),
        label(label),
        cls(CLASS::BITMASK),
        access(access),
        data_type(data_type),
        enum_def(enum_def) {}
  /// @brief Constructor for ENUM registers definitions
  REG_DEF(register_id_t register_id, const char *label, ACCESS access, ENUM_DEF *enum_def)
      : register_id(register_id),
        label(label),
        cls(CLASS::ENUM),
        access(access),
        data_type(DATA_TYPE::UN8),
        enum_def(enum_def) {}
  /// @brief Constructor for NUMERIC registers definitions
  REG_DEF(register_id_t register_id, const char *label, ACCESS access, DATA_TYPE data_type, UNIT unit, SCALE scale,
          SCALE text_scale)
      : register_id(register_id),
        label(label),
        cls(CLASS::NUMERIC),
        access(access),
        data_type(data_type),
        unit(unit),
        scale(scale),
        text_scale(text_scale) {}

 protected:
};

/// @brief Descriptor struct for TEXT frame records
struct TEXT_DEF {
  const char *label;
  const REG_DEF::TYPE register_type;

  // Constructor used when register_type is a valid mapping to a REG_DEF
  TEXT_DEF(const char *label, REG_DEF::TYPE register_type) : label(label), register_type(register_type) {}

  // Constructor for default unknown/untyped field
  TEXT_DEF() : label(nullptr), register_type(REG_DEF::TYPE::TYPE_COUNT) {}

  bool operator<(const char *label) const { return strcmp(this->label, label) < 0; }
  static const TEXT_DEF DEFS[];
  static const TEXT_DEF *find_label(const char *label);
  static const TEXT_DEF *find_type(REG_DEF::TYPE register_type);
};

#pragma pack(pop)

}  // namespace m3_ve_reg