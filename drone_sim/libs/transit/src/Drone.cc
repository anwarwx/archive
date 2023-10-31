#define _USE_MATH_DEFINES
#include "Drone.h"

#include <cmath>
#include <limits>

#include "AstarStrategy.h"
#include "BeelineStrategy.h"
#include "DfsStrategy.h"
#include "DijkstraStrategy.h"
#include "JumpDecorator.h"
#include "SpinDecorator.h"

Drone::Drone(JsonObject& obj) : details(obj) {
  JsonArray pos(obj["position"]);
  position = {pos[0], pos[1], pos[2]};
  JsonArray dir(obj["direction"]);
  direction = {dir[0], dir[1], dir[2]};

  speed = obj["speed"];

  available = true;
}

Drone::~Drone() {
  // Delete dynamically allocated variables
  delete graph;
  delete nearestEntity;
  delete toRobot;
  delete toFinalDestination;
  delete toPublisher;
}

void Drone::GetNearestEntity(std::vector<IEntity*> scheduler) {
  float minDis = std::numeric_limits<float>::max();
  for (auto entity : scheduler) {
    if (entity->GetAvailability()) {
      float disToEntity = this->position.Distance(entity->GetPosition());
      if (disToEntity <= minDis) {
        minDis = disToEntity;
        nearestEntity = entity;
      }
    }
  }

  if (nearestEntity) {
    std::string ret = nearestEntity->GetDetails()["name"];
    robotName = ret;

    // set availability to the nearest entity
    nearestEntity->SetAvailability(false);
    available = false;
    pickedUp = false;

    destination = nearestEntity->GetPosition();
    Vector3 finalDestination = nearestEntity->GetDestination();

    toRobot = new BeelineStrategy(position, destination);

    std::string strat = nearestEntity->GetStrategyName();
    if (strat == "astar")
      toFinalDestination =
        new JumpDecorator(new AstarStrategy
        (destination, finalDestination, graph));
    else if (strat == "dfs")
      toFinalDestination =
        new SpinDecorator(new JumpDecorator
        (new DfsStrategy(destination, finalDestination, graph)));
    else if (strat == "dijkstra")
      toFinalDestination =
        new JumpDecorator(new SpinDecorator
        (new DijkstraStrategy(destination, finalDestination, graph)));
    else
      toFinalDestination = new BeelineStrategy(destination, finalDestination);
  }
}

void Drone::Update(double dt, std::vector<IEntity*> scheduler) {
  if (available) {
    GetNearestEntity(scheduler);
  }

  UpdateHelper();

  if (toRobot) {
    toRobot->Move(this, dt);
    toPublisher->SendEvents(
      name + ": Picking up the robot \"" + robotName + "\"\n\n");

    if (toRobot->IsCompleted()) {
      toPublisher->SendEvents(
        name + ": Picked up the robot \"" + robotName + "\"\n\n");
      delete toRobot;
      toRobot = nullptr;
      pickedUp = true;
      run = true;
    }
  } else if (toFinalDestination) {
    toFinalDestination->Move(this, dt);

    if (nearestEntity && pickedUp) {
      toPublisher->SendEvents(
        name + ": Delivering the robot \"" + robotName + "\"\n\n");
      nearestEntity->SetPosition(position);
      nearestEntity->SetDirection(direction);
    }

    if (toFinalDestination->IsCompleted()) {
      dynamic_cast<Robot*>(nearestEntity)->AvailableToDelete();
      toPublisher->SendEvents(
        name + ": Delivered the robot \"" + robotName + "\"\n\n");
      delete toFinalDestination;
      toFinalDestination = nullptr;
      nearestEntity = nullptr;
      available = true;
      pickedUp = false;
      run = true;
    }
  }
}

void Drone::Rotate(double angle) {
  Vector3 dirTmp = direction;
  direction.x = dirTmp.x * std::cos(angle) - dirTmp.z * std::sin(angle);
  direction.z = dirTmp.x * std::sin(angle) + dirTmp.z * std::cos(angle);
}

void Drone::Jump(double height) {
  if (goUp) {
    position.y += height;
    jumpHeight += height;
    if (jumpHeight > 5) {
      goUp = false;
    }
  } else {
    position.y -= height;
    jumpHeight -= height;
    if (jumpHeight < 0) {
      goUp = true;
    }
  }
}

void Drone::UpdateHelper() {
  if (GLOBAL_WEATHER->Forecast() == "snow") {
    if (toRobot) {
      if (run) {
        delete toRobot;
        toRobot = new AstarStrategy(position, destination, graph);
        emergency = true;
        run = false;
      }
      if (emergency && GLOBAL_WEATHER->IsCompleted()) {
        delete toRobot;
        toRobot = new BeelineStrategy(position, destination);
        emergency = false;
        run = true;
      }
    }
  } else if (GLOBAL_WEATHER->Forecast() == "hot") {
    if (!toRobot && toFinalDestination) {
      if (run) {
        delete toFinalDestination;
        toFinalDestination =
          new BeelineStrategy(position, nearestEntity->GetDestination());
        emergency = true;
        run = false;
      }
      if (emergency && GLOBAL_WEATHER->IsCompleted()) {
        delete toFinalDestination;
        toFinalDestination = new SpinDecorator(
          new AstarStrategy(position, nearestEntity->GetDestination(), graph));
        emergency = false;
        run = true;
      }
    }
  }
}
