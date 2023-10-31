#include "WeatherGFX.h"

WeatherGFX::WeatherGFX(JsonObject& obj) : details(obj) {
  JsonArray pos(obj["position"]);
  position = {pos[0], pos[1], pos[2]};
  JsonArray dir(obj["direction"]);
  direction = {dir[0], dir[1], dir[2]};
  speed = obj["speed"];
}

void WeatherGFX::Update(double dt, std::vector<IEntity*> IGNORE) {
  if (toDestination) {
    if (toDestination->IsCompleted()) {
      destination = {Random(-1400, 1500), position.y, Random(-800, 800)};
      toDestination = new BeelineStrategy(position, destination);
    } else {
      toDestination->Move(this, dt);
    }
  } else {
    destination = {Random(-1400, 1500), position.y, Random(-800, 800)};
    toDestination = new BeelineStrategy(position, destination);
  }
}
