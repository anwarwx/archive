#include "SimulationModel.h"

#include "DroneFactory.h"
#include "RobotFactory.h"
#include "HumanFactory.h"
#include "SatelliteFactory.h"

SimulationModel::SimulationModel(IController& controller)
    : controller(controller) {
  compFactory = new CompositeFactory();
  AddFactory(new DroneFactory());
  AddFactory(new RobotFactory());
  AddFactory(new HumanFactory());
  AddFactory(new SatelliteFactory());
  (GLOBAL_WEATHER->GetPublisher())->Subscribe(&controller);
}

SimulationModel::~SimulationModel() {
  // Delete dynamically allocated variables
  for (int i = 0; i < entities.size(); i++) {
    delete entities[i];
  }
  for (int i = 0; i < scheduler.size(); i++) {
    delete scheduler[i];
  }
  delete graph;
  delete compFactory;
}

void SimulationModel::CreateEntity(JsonObject& entity) {
  if (GLOBAL_WEATHER->CreateGFX(entity, controller)) { return; }

  std::string type = entity["type"];

  // humans aren't spawned correctly when there speed is 0
  if (GLOBAL_WEATHER->Forecast() != "normal" &&
      GLOBAL_WEATHER->Forecast() != "rain") {
    if (type == "human") {
      std::cout << "[!] Bad weather condition, " <<
      entity["name"] << " not added!" << std::endl;
      return;
    }
  }

  std::string name = entity["name"];
  JsonArray position = entity["position"];
  std::cout << name << ": " << position << std::endl;

  IEntity* myNewEntity = compFactory->CreateEntity(entity);
  myNewEntity->SetGraph(graph);

  // Call AddEntity to add it to the view
  controller.AddEntity(*myNewEntity);
  entities.push_back(myNewEntity);

  if (type == "drone") {
    IPublisher* eventManager = new DronePublisher();
    eventManager->Subscribe(&controller);
    myNewEntity->SetPublisher(eventManager);
  }
}

/// Schedules a trip for an object in the scene
void SimulationModel::ScheduleTrip(JsonObject& details) {
  std::string name = details["name"];
  JsonArray start = details["start"];
  JsonArray end = details["end"];
  std::cout << name << ": " << start << " --> " << end << std::endl;

  for (auto entity : entities) {  // Add the entity to the scheduler
    JsonObject detailsTemp = entity->GetDetails();
    std::string nameTemp = detailsTemp["name"];
    std::string typeTemp = detailsTemp["type"];
    if (name.compare(nameTemp) == 0 && typeTemp.compare("robot") == 0 &&
        entity->GetAvailability()) {
      std::string strategyName = details["search"];
      entity->SetDestination(Vector3(end[0], end[1], end[2]));
      entity->SetStrategyName(strategyName);
      scheduler.push_back(entity);
      break;
    }
  }
  controller.SendEventToView("TripScheduled", details);
}

/// Updates the simulation
void SimulationModel::Update(double dt) {
  GLOBAL_WEATHER->Update(dt, entities);
  GLOBAL_WEATHER->UpdateGFX(dt, controller);

  for (int i = 0; i < entities.size(); i++) {
    GLOBAL_WEATHER->Run(i, controller, entities);
    entities[i]->Update(dt, scheduler);
    controller.UpdateEntity(*entities[i]);
  }
  GLOBAL_WEATHER->Reverse(controller, entities);
}

void SimulationModel::AddFactory(IEntityFactory* factory) {
  compFactory->AddFactory(factory);
}
