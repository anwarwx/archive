#define _USE_MATH_DEFINES
#include "Satellite.h"

#include <cmath>
#include <limits>

#include "BeelineStrategy.h"

Satellite::Satellite(JsonObject& obj) : details(obj) {
  JsonArray pos(obj["position"]);
  position = {pos[0], pos[1], pos[2]};
  JsonArray dir(obj["direction"]);
  direction = {dir[0], dir[1], dir[2]};

  speed = obj["speed"];
}

Satellite::~Satellite() {
  // Delete dynamically allocated variables
  delete graph;
}

void Satellite::CreateNewDestination() {
  destination = {Random(-1400, 1500), position.y, Random(-800, 800)};
  toDestination = new BeelineStrategy(position, destination);
}

void Satellite::Rotate(double angle) {
  Vector3 dirTmp = direction;
  direction.x = dirTmp.x * std::cos(angle) - dirTmp.z * std::sin(angle);
  direction.z = dirTmp.x * std::sin(angle) + dirTmp.z * std::cos(angle);
}

void Satellite::Update(double dt, std::vector<IEntity*> scheduler) {
    if (toDestination) {
        if (toDestination->IsCompleted()) {
          CreateNewDestination();
        } else {
          toDestination->Move(this, dt);
        }
    } else {
        CreateNewDestination();
    }
}
