#include "register.h"

namespace esphome {
namespace m3_vedirect {

Register::build_entity_func_t Register::BUILD_ENTITY_FUNC[Platform_COUNT] = {};

void Register::dynamic_init_entity_(EntityBase *entity, const char *name, const char *object_id,
                                    const char *manager_name, const char *manager_id) {
  if (manager_name) {
    char *entity_name = new char[strlen(manager_name) + strlen(name) + 2];
    sprintf(entity_name, "%s.%s", manager_name, name);
    name = entity_name;
  }
  entity->set_name(name);
  char *entity_object_id = new char[strlen(manager_id) + strlen(object_id) + 2];
  sprintf(entity_object_id, "%s_%s", manager_id, object_id);
  entity->set_object_id(entity_object_id);
}

void Register::update_platforms() {
  // This code will run after initial setup and will check if any platform
  // is missing. When this happens, it'll try to use the most close platform
  // implementation available generally falling back to TextSensor which is
  // able to manage any kind of data though. If nothing better, it will just
  // install Register::build_entity which will just work as a stub since Register
  // itself doesn't implement any useful behavior for parsing
  if (!BUILD_ENTITY_FUNC[TextSensor]) {
    BUILD_ENTITY_FUNC[TextSensor] = Register::build_entity;
  }
  if (!BUILD_ENTITY_FUNC[Select]) {
    BUILD_ENTITY_FUNC[Select] = BUILD_ENTITY_FUNC[TextSensor];
  }
  if (!BUILD_ENTITY_FUNC[BinarySensor]) {
    BUILD_ENTITY_FUNC[BinarySensor] = BUILD_ENTITY_FUNC[TextSensor];
  }
  if (!BUILD_ENTITY_FUNC[Switch]) {
    BUILD_ENTITY_FUNC[Switch] = BUILD_ENTITY_FUNC[BinarySensor];
  }
  if (!BUILD_ENTITY_FUNC[Sensor]) {
    BUILD_ENTITY_FUNC[Sensor] = BUILD_ENTITY_FUNC[TextSensor];
  }
  if (!BUILD_ENTITY_FUNC[Number]) {
    BUILD_ENTITY_FUNC[Number] = BUILD_ENTITY_FUNC[Sensor];
  }
}

Register *Register::cascade_dispatcher_(Register *hex_register) {
  auto dispatcher = new RegisterDispatcher(this);
  return dispatcher->cascade_dispatcher_(hex_register);
}

const char *NumericRegister::UNIT_TO_DEVICE_CLASS[REG_DEF::UNIT::UNIT_COUNT] = {
    nullptr, nullptr,  "current", "voltage",  "apparent_power", "power",
    nullptr, "energy", "battery", "duration", "temperature",    "temperature",
};

}  // namespace m3_vedirect
}  // namespace esphome