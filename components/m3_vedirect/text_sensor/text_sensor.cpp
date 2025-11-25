#include "text_sensor.h"
#include "esphome/core/application.h"
#if ESPHOME_VERSION_CODE < VERSION_CODE(2025, 11, 0)
#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif
#endif

#include "../manager.h"

#include <cinttypes>

namespace esphome {
namespace m3_vedirect {

Register *TextSensor::build_entity(Manager *manager, const REG_DEF *reg_def, const char *name) {
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 8, 0)
  if (App.get_text_sensors().size() >= ESPHOME_ENTITY_TEXT_SENSOR_COUNT) {
    return Register::drop_platform(manager, Platform::TextSensor);
  }
#endif
  auto entity = new TextSensor(manager);
  manager->init_entity(entity, reg_def, name);
  App.register_text_sensor(entity);
#if ESPHOME_VERSION_CODE < VERSION_CODE(2025, 11, 0)
// See https://github.com/esphome/esphome/pull/11772
#if defined(USE_API)
  entity->add_on_state_callback(
      [entity](const std::string &state) { api::global_api_server->on_text_sensor_update(entity, state); });
#endif
#endif
  return entity;
}

void TextSensor::link_disconnected_() {
  if (this->has_state()) {
    this->raw_value_ = BITMASK_DEF::VALUE_UNKNOWN;
    this->publish_state("unknown");
    this->set_has_state(false);
  }
}

void TextSensor::init_reg_def_() {
  switch (this->reg_def_->cls) {
    case REG_DEF::CLASS::BITMASK:
#if defined(VEDIRECT_USE_HEXFRAME)
      this->parse_hex_ = parse_hex_bitmask_;
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
      this->parse_text_ = parse_text_bitmask_;
#endif
      break;
    case REG_DEF::CLASS::ENUM:
#if defined(VEDIRECT_USE_HEXFRAME)
      this->parse_hex_ = parse_hex_enum_;
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
      this->parse_text_ = parse_text_enum_;
#endif
      break;
    case REG_DEF::CLASS::STRING:
#if defined(VEDIRECT_USE_HEXFRAME)
      this->parse_hex_ = parse_hex_string_;
#endif
      break;
    case REG_DEF::CLASS::VOID:
      if (this->reg_def_->register_id == 0x0102) {
        // APP_VER (firmware version) register
#if defined(VEDIRECT_USE_HEXFRAME)
        this->parse_hex_ = parse_hex_app_ver_;
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
        this->parse_text_ = parse_text_app_ver_;
#endif
      }
      break;
    default:
      break;
  }
}

void TextSensor::parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value) {
  if (this->raw_value_ != bitmask_value) {
    this->raw_value_ = bitmask_value;
    std::string state;
    ENUM_DEF *enum_def = this->reg_def_->enum_def;
    for (uint8_t bit = 0; bitmask_value; ++bit) {
      if (bitmask_value & 0x01) {
        if (state.size())
          state += ",";
        state += enum_def->get_lookup(bit).lookup_def->label;
      }
      bitmask_value >>= 1;
    }
    this->publish_state(state);
  }
}

void TextSensor::parse_enum_(ENUM_DEF::enum_t enum_value) {
  if (this->raw_value_ != enum_value) {
    this->raw_value_ = enum_value;
    this->publish_state(std::string(this->reg_def_->enum_def->get_lookup(enum_value).lookup_def->label));
  }
}

void TextSensor::parse_string_(const char *string_value) {
  if (strcmp(this->raw_state.c_str(), string_value)) {
    this->raw_value_ = BITMASK_DEF::VALUE_UNKNOWN;
    this->publish_state(std::string(string_value));
  }
}

#if defined(VEDIRECT_USE_HEXFRAME)
void TextSensor::parse_hex_default_(Register *hex_register, const RxHexFrame *hex_frame) {
  char hex_value[RxHexFrame::ALLOCATED_ENCODED_SIZE];
  if (hex_frame->data_to_hex(hex_value, RxHexFrame::ALLOCATED_ENCODED_SIZE)) {
    static_cast<TextSensor *>(hex_register)->parse_string_(hex_value);
  }
}

void TextSensor::parse_hex_bitmask_(Register *hex_register, const RxHexFrame *hex_frame) {
  // BITMASK registers have storage up to 4 bytes
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  static_cast<TextSensor *>(hex_register)->parse_bitmask_(hex_frame->safe_data_u32());
}

void TextSensor::parse_hex_enum_(Register *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<TextSensor *>(hex_register)->parse_enum_(hex_frame->data_t<ENUM_DEF::enum_t>());
}

void TextSensor::parse_hex_string_(Register *hex_register, const RxHexFrame *hex_frame) {
  static_cast<TextSensor *>(hex_register)->parse_string_(hex_frame->data_str());
}

void TextSensor::parse_hex_app_ver_(Register *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 3, "HexFrame storage might lead to access overflow");
  // Parsing of HEX register for fw version will lead to a representation that might be
  // different from that carried in the TEXT frame, at least for non-release versions.
  uint8_t major = *(hex_frame->data_begin() + 2);
  uint8_t minor = *(hex_frame->data_begin() + 1);
  uint8_t sw_type = major & 0xC0;
  major &= 0x3F;
  char buf[32];
  switch (sw_type) {
    case 0x40:  // release
      sprintf(buf, "%hhu.%.2hhu", major, minor);
      break;
    case 0xC0:  // beta
      sprintf(buf, "%hhu.%.2hhu-beta", major, minor);
      break;
    case 0x80:  // tester
      sprintf(buf, "%hhu.%.2hhu-tester", major, minor);
      break;
    // case 0x00:  // bootloader
    default:
      sprintf(buf, "%hhu.%.2hhu-bootloader", major, minor);
      break;
  }
  static_cast<TextSensor *>(hex_register)->parse_string_(buf);
}
#endif  // defined(VEDIRECT_USE_HEXFRAME)

#if defined(VEDIRECT_USE_TEXTFRAME)
void TextSensor::parse_text_default_(Register *hex_register, const char *text_value) {
  static_cast<TextSensor *>(hex_register)->parse_string_(text_value);
}

void TextSensor::parse_text_bitmask_(Register *hex_register, const char *text_value) {
  // When parsing text records for BITMASK-like values, the TEXT protocol might sometime carry
  // decimal based values and sometimes hexadecimal base values. This should be automatically
  // handled by strtoumax
  char *endptr;
  BITMASK_DEF::bitmask_t bitmask_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<TextSensor *>(hex_register)->parse_bitmask_(bitmask_value);
  } else {
    static_cast<TextSensor *>(hex_register)->parse_string_(text_value);
  }
}

void TextSensor::parse_text_enum_(Register *hex_register, const char *text_value) {
  char *endptr;
  ENUM_DEF::enum_t enum_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<TextSensor *>(hex_register)->parse_enum_(enum_value);
  } else {
    static_cast<TextSensor *>(hex_register)->parse_string_(text_value);
  }
}

void TextSensor::parse_text_app_ver_(Register *hex_register, const char *text_value) {
  // Here we expect to parse either 'FW' or 'FWE' text records. We'll use strlen to decide how to interpret
  // the payload (see https://www.victronenergy.com/upload/documents/VE.Direct-Protocol-3.33.pdf)
  auto len = strlen(text_value);
  char buf[14];
  switch (len) {
    case 3:
      // '208' -> fw: 2.08
      buf[0] = text_value[0];
      buf[1] = '.';
      buf[2] = text_value[1];
      buf[3] = text_value[2];
      buf[4] = 0;
      break;
    case 4:
      // 'C208' -> fw: 2.08.beta.C
      buf[0] = text_value[1];
      buf[1] = '.';
      buf[2] = text_value[2];
      buf[3] = text_value[3];
      strcpy(buf + 4, "-beta-");
      buf[10] = text_value[0];
      buf[11] = 0;
      break;
    case 5:
    handle_5:
      // '208FF' -> fw: 2.08 (official release)
      buf[0] = text_value[0];
      buf[1] = '.';
      buf[2] = text_value[1];
      buf[3] = text_value[2];
      if ((text_value[3] == 'F') && (text_value[4] == 'F')) {
        buf[4] = 0;
      } else {
        strcpy(buf + 4, "-beta-");
        buf[10] = text_value[3];
        buf[11] = text_value[4];
        buf[12] = 0;
      }
      break;
    case 6:
      if (text_value[0] == '0') {
        ++text_value;
        goto handle_5;
      }
      buf[0] = text_value[0];
      buf[1] = text_value[1];
      buf[2] = '.';
      buf[3] = text_value[2];
      buf[4] = text_value[3];
      if ((text_value[4] == 'F') && (text_value[5] == 'F')) {
        buf[5] = 0;
      } else {
        strcpy(buf + 5, "-beta-");
        buf[11] = text_value[4];
        buf[12] = text_value[5];
        buf[13] = 0;
      }
      break;
    default:
      // unknown format..just forward 'as is'
      static_cast<TextSensor *>(hex_register)->parse_string_(text_value);
      return;
  }
  static_cast<TextSensor *>(hex_register)->parse_string_(buf);
}
#endif  // defined(VEDIRECT_USE_TEXTFRAME)

}  // namespace m3_vedirect
}  // namespace esphome
