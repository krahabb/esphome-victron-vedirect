#include "register.h"
#include "manager.h"

// Static initialization of the platform build functions table:
// In case the corresponding platform is not included in the project
// we'll try falling back to a reasonable substitute which most of the times
// will finally resolve to TextSensor. If nothing else, we'll use Register::build_entity
// which will just create a plain Register object which has no useful behavior
// but will work as a (void) sink for data dispatching.
#if defined(USE_TEXT_SENSOR)
#include "text_sensor/text_sensor.h"
#define TextSensorBuildF TextSensor::build_entity
#else
#define TextSensorBuildF Register::build_entity
#endif
#if defined(USE_SELECT)
#include "select/select.h"
#define SelectBuildF Select::build_entity
#else
#define SelectBuildF TextSensorBuildF
#endif
#if defined(USE_SENSOR)
#include "sensor/sensor.h"
#define SensorBuildF Sensor::build_entity
#else
#define SensorBuildF TextSensorBuildF
#endif
#if defined(USE_NUMBER)
#include "number/number.h"
#define NumberBuildF Number::build_entity
#else
#define NumberBuildF SensorBuildF
#endif
#if defined(USE_BINARY_SENSOR)
#include "binary_sensor/binary_sensor.h"
#define BinarySensorBuildF BinarySensor::build_entity
#else
#define BinarySensorBuildF TextSensorBuildF
#endif
#if defined(USE_SWITCH)
#include "switch/switch.h"
#define SwitchBuildF Switch::build_entity
#else
#define SwitchBuildF BinarySensorBuildF
#endif

namespace esphome {
namespace m3_vedirect {

Register::build_entity_func_t Register::BUILD_ENTITY_FUNC[Platform_COUNT] = {
    BinarySensorBuildF, NumberBuildF, SelectBuildF, SensorBuildF, SwitchBuildF, TextSensorBuildF,
};
const Register::Platform Register::REGDEF_FACTORY_MAP[REG_DEF::CLASS::CLASS_COUNT][REG_DEF::ACCESS::ACCESS_COUNT] = {
    // VOID
    {TextSensor, TextSensor, TextSensor},
    // BITMASK
    {TextSensor, TextSensor, TextSensor},
    // BOOLEAN
    {BinarySensor, BinarySensor, Switch},
    // ENUM
    {TextSensor, TextSensor, Select},
    // NUMERIC
    {Sensor, Sensor, Number},
    // STRING
    {TextSensor, TextSensor, TextSensor},
};

Register *Register::auto_create(Manager *manager, const REG_DEF *reg_def) {
  ESP_LOGD(manager->get_logtag(), "Auto-Creating HEX register: %04X", (int) reg_def->register_id);
  Register *reg =
      BUILD_ENTITY_FUNC[REGDEF_FACTORY_MAP[reg_def->cls][reg_def->access]](manager, reg_def, reg_def->label);
  manager->init_register(reg, reg_def);
  return reg;
}

Register *Register::drop_platform(Manager *manager, Platform platform) {
  ESP_LOGW(manager->get_logtag(), "Reached maximum entities count for {Platform:%d}. Dropping entity factory",
           platform);
  // We'll scan the whole set since a specific platform might have been installed
  // as a fallback for other platforms.
  build_entity_func_t drop_build_entity_func = BUILD_ENTITY_FUNC[platform];
  for (int p = 0; p < Platform_COUNT; ++p) {
    if (BUILD_ENTITY_FUNC[p] == drop_build_entity_func) {
      BUILD_ENTITY_FUNC[p] = Register::build_entity;
    }
  }
  return Register::build_entity(nullptr, nullptr, nullptr);
}

Register *Register::cascade_dispatcher_(Register *_register) {
  auto dispatcher = new RegisterDispatcher(this);
  return dispatcher->cascade_dispatcher_(_register);
}

#if defined(VEDIRECT_USE_HEXFRAME)
void WritableRegister::request_set_(uint32_t value, std::function<void(const HexFrame *, uint8_t)> &&callback) {
  this->manager->request_set(this->reg_def_->register_id, &value,
                             HEXFRAME::DATA_TYPE_TO_SIZE[this->reg_def_->data_type], std::move(callback));
}
#endif

const char *NumericRegister::UNIT_TO_DEVICE_CLASS[REG_DEF::UNIT::UNIT_COUNT] = {
    nullptr,  nullptr,   "current", "voltage",  "apparent_power", "power",       nullptr,
    "energy", "battery", nullptr,   "duration", "duration",       "temperature", "temperature",
};

}  // namespace m3_vedirect
}  // namespace esphome
