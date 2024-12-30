#include "ve_reg_def.h"
#include <algorithm>
#include <cstring>

namespace m3_ve_reg {

const char *ENUM_DEF::lookup_label(enum_t value) {
  auto lookup_def_it = std::lower_bound(this->LOOKUPS.begin(), this->LOOKUPS.end(), value);
  return (lookup_def_it != this->LOOKUPS.end()) && (lookup_def_it->value == value) ? lookup_def_it->label : nullptr;
}

const ENUM_DEF::LOOKUP_DEF *ENUM_DEF::lookup_value(const char *label) {
  for (auto lookup_def_it = this->LOOKUPS.begin(); lookup_def_it != this->LOOKUPS.end(); ++lookup_def_it) {
    if (0 == strcmp(lookup_def_it->label, label))
      return &*lookup_def_it;
  }
  return nullptr;
}

ENUM_DEF::LOOKUP_RESULT ENUM_DEF::get_lookup(enum_t value) {
  LOOKUP_RESULT result;
  auto lookup_def_it = std::lower_bound(this->LOOKUPS.begin(), this->LOOKUPS.end(), value);
  result.index = lookup_def_it - this->LOOKUPS.begin();
  if ((lookup_def_it == this->LOOKUPS.end()) || (lookup_def_it->value != value)) {
    char *label = new char[5];
    sprintf(label, "0x%02X", (int) value);
    this->LOOKUPS.insert(lookup_def_it, {value, label});
    result.lookup_def = &this->LOOKUPS[result.index];
    result.added = true;
  } else {
    result.lookup_def = &*lookup_def_it;
    result.added = false;
  }
  return result;
}

const char *REG_DEF::UNITS[UNIT::UNIT_COUNT] = {
    nullptr, nullptr, "A", "V", "VA", "W", "Ah", "kWh", "%", "min", "°C", "°C",
};
const float REG_DEF::SCALE_TO_SCALE[SCALE::SCALE_COUNT] = {
    1.f,    // S_1,
    .1f,    // S_0_1,
    .01f,   // S_0_01,
    .001f,  // S_0_001,
    .25f,   // S_0_25,
};

// define the enum helpers structs for ENUM registers
#define _ENUMS_LOOKUP_ITEM(enum, value) \
  { value, #enum }
#define _DEFINE_ENUMS(cls, register_id, label, ...) \
  cls##_DEF VE_REG_##label##_##cls##_DEF = {{cls##_##label(_ENUMS_LOOKUP_ITEM)}};
#define DEFINE_ENUMS(flavor, cls, register_id, label, ...) \
  IF(DEF_##flavor)(IF(_DEF_ENUM_##cls)(_DEFINE_ENUMS(cls, register_id, label, ...)))

REGISTERS_COMMON(DEFINE_ENUMS)

// define the registers definitions (will be stored in REG_DEF::DEFS)
#define DEFINE_REG_DEF_VOID(register_id, label, access) {register_id, #label, CLASS::VOID, REG_DEF::access},
#define DEFINE_REG_DEF_BOOLEAN(register_id, label, access) {register_id, #label, CLASS::BOOLEAN, REG_DEF::access},
#define DEFINE_REG_DEF_BITMASK(register_id, label, access, type) \
  {register_id, #label, REG_DEF::access, HEXFRAME::DATA_TYPE_OF<type>(), &VE_REG_##label##_BITMASK_DEF},
#define DEFINE_REG_DEF_BITMASK_S(register_id, label, access, type, bitmask_label) \
  {register_id, #label, REG_DEF::access, HEXFRAME::DATA_TYPE_OF<type>(), &VE_REG_##bitmask_label##_BITMASK_DEF},
#define DEFINE_REG_DEF_ENUM(register_id, label, access) \
  {register_id, #label, REG_DEF::access, &VE_REG_##label##_ENUM_DEF},
#define DEFINE_REG_DEF_ENUM_S(register_id, label, access, enum_label) \
  {register_id, #label, REG_DEF::access, &VE_REG_##enum_label##_ENUM_DEF},
#define DEFINE_REG_DEF_NUMERIC(register_id, label, access, type, unit, scale, text_scale) \
  {register_id,   #label,         REG_DEF::access,    HEXFRAME::DATA_TYPE_OF<type>(), \
   REG_DEF::unit, REG_DEF::scale, REG_DEF::text_scale},
#define DEFINE_REG_DEF_STRING(register_id, label, access) {register_id, #label, CLASS::STRING, REG_DEF::access},
#define DEFINE_REG_DEF(flavor, cls, ...) IF(DEF_##flavor)(DEFINE_REG_DEF_##cls(__VA_ARGS__))
const REG_DEF REG_DEF::DEFS[REG_DEF::TYPE::TYPE_COUNT] = {REGISTERS_COMMON(DEFINE_REG_DEF)};

const REG_DEF *REG_DEF::find_register_id(register_id_t register_id) {
  const REG_DEF *reg_def_end = DEFS + ARRAY_COUNT(DEFS);
  auto reg_def_it = std::lower_bound(DEFS, reg_def_end, register_id);
  return (reg_def_it != reg_def_end) && (reg_def_it->register_id == register_id) ? reg_def_it : nullptr;
}

#define DEFINE_TEXT_DEF_REG(label, register_type) {label, REG_DEF::TYPE::register_type},
#define DEFINE_TEXT_DEF(flavor, cls, ...) IF(DEF_##flavor)(DEFINE_TEXT_DEF_REG(__VA_ARGS__))
const TEXT_DEF TEXT_DEF::DEFS[] = {TEXTRECORDS(DEFINE_TEXT_DEF)};

const TEXT_DEF *TEXT_DEF::find_label(const char *label) {
  const TEXT_DEF *it_end = DEFS + ARRAY_COUNT(DEFS);
  auto it = std::lower_bound(DEFS, it_end, label);
  return (it != it_end) && (strcmp(it->label, label) == 0) ? it : nullptr;
}

const TEXT_DEF *TEXT_DEF::find_type(REG_DEF::TYPE register_type) {
  for (auto &text_def : DEFS) {
    if (text_def.register_type == register_type)
      return &text_def;
  }
  return nullptr;
}

}  // namespace m3_ve_reg