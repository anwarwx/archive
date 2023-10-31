#include "SatelliteFactory.h"

IEntity* SatelliteFactory::CreateEntity(JsonObject& entity) {
  std::string type = entity["type"];
  if (type.compare("satellite") == 0) {
    std::cout << "Satellite Created" << std::endl;
    return new Satellite(entity);
  }
  return nullptr;
}
