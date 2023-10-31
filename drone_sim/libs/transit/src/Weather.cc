#include "Weather.h"
#include <algorithm>

Weather::~Weather() {
  for (auto const& [key, model] : GFX) {
    delete model;
  }
  GFX.clear();
  delete toPublisher;
}

Weather* Weather::GetInstance() {
  static Weather GLOBAL_WEATHER;
  return &GLOBAL_WEATHER;
}

/*
  Weather::Update is called from SimulationModel::Update,
  which makes this is a repeated call.

  Weather::Update calls each type of weather, which goes on and
  applies it's own unique effect to the simulation, only if the
  forecast condition is met.

  Upon weather completion:
  (1) if any changes were made, then we revert them.
  (2) reset the time,
  (3) get a new weather based on it's probability
*/
void Weather::Update(double dt, std::vector<IEntity*>& entities) {
  if (IsCompleted()) {
    if (Forecast() != "normal") {
      Revert(entities);
    }
    time = 0.0;
    index = val(GEN);
    run = true;
  }

  Normal();
  Snow(entities);
  Tornado(entities);
  Rain();
  Hot(entities);
  Hurricane(entities);

  time += dt;
}

void Weather::Normal() {
  if (Forecast() == "normal") {
    if (run) {
      toPublisher->SendEvents("Weather: Default \U0001F601\n\n");
      run = false;
    }
  }
}

/*
  Weather::Snow applies a "heavy snow" weather condition,
  
  All entities, except for humans, are slowed down to 50%
  of their natural speed.

  Whereas, human entities stop moving. We do this for safety
  pre-cautions. We wouldn't want our beloved humans to get hurt
  by slipping on ice.
*/
void Weather::Snow(std::vector<IEntity*>& entities) {
  if (Forecast() == "snow") {
    if (run) {
      toPublisher->SendEvents("Weather: Heavy Snow! \U0001F328\n\n");
      run = false;
    }
    // all entities speed cuts to half,
    for (int i = 0; i < entities.size(); ++i) {
      float speed = entities[i]->GetSpeed();
      if (prev[entities[i]->GetId()] == 0) {
        if (speed > 10.0) {
          prev[entities[i]->GetId()] = speed;
          entities[i]->SetSpeed(speed / 2.0);
        }
      }
      // except humans => stop moving
      details = entities[i]->GetDetails();
      std::string type = details["type"];
      if (type == "human") {
        if (speed > 1) {
          prev[entities[i]->GetId()] = speed;
          entities[i]->SetSpeed(0.0);
        }
      }
    }
  }
}

/*
  Weather::Tornado gives a tornado warning.

  All entities are to stop moving, take cover and wait for
  the tornado to pass.

  Our very own Satellite, is to go back into orbit, since
  replacing one requires a hefty sum. We wouldn't want it to 
  get destroyed now would we?
*/
void Weather::Tornado(std::vector<IEntity*>& entities) {
  if (Forecast() == "tornado") {
    if (run) {
      toPublisher->SendEvents("Weather: Tornado Warning! \U0001F32A\n\n");
      run = false;
    }
    // all entities stop moving
    for (int i = 0; i < entities.size(); ++i) {
      float speed = entities[i]->GetSpeed();
      if (speed > 1) {
        prev[entities[i]->GetId()] = speed;
        entities[i]->SetSpeed(0.0);
      }
    }
  }
}

void Weather::Rain() {
  if (Forecast() == "rain") {
    if (run) {
      toPublisher->SendEvents("Weather: Light Rain \U0001F327\n\n");
      run = false;
    }
  }
}

/*
  Weather::Hot applies a "blazing hot" weather condition,

  All entities, except satellite, are to speed up. We want
  to get to our destinations as soon as possible, because this
  weather isn't to play around with.
*/
void Weather::Hot(std::vector<IEntity*>& entities) {
  if (Forecast() == "hot") {
    if (run) {
      toPublisher->SendEvents("Weather: Blazing Hot! \U0001FAE0\n\n");
      run = false;
    }
    // entities, except satellite, speed gets doubled
    for (int i = 0; i < entities.size(); ++i) {
      details = entities[i]->GetDetails();
      std::string type = details["type"];
      if (type != "satellite") {
        float speed = entities[i]->GetSpeed();
        if (prev.count(entities[i]->GetId()) == 0) {
          prev[entities[i]->GetId()] = speed;
          entities[i]->SetSpeed(speed * 2.0);
        }
      }
    }
  }
}

void Weather::Hurricane(std::vector<IEntity*>& entities) {
  if (Forecast() == "hurricane") {
    if (run) {
      toPublisher->SendEvents("Weather: Hurricane Warning! \U0001F6A8\n\n");
      run = false;
    }
    // all entities stop moving
    for (int i = 0; i < entities.size(); ++i) {
      float speed = entities[i]->GetSpeed();
      if (speed > 1) {
        prev[entities[i]->GetId()] = speed;
        entities[i]->SetSpeed(0.0);
      }
    }
  }
}

/*
  Weather::Revert uses std::map which has all the entities
  default speeds stored into it. Before any changes were made
  to the entities we saved their previous info and now to revert
  back those changes we just reapply their defaults.
*/
void Weather::Revert(std::vector<IEntity*>& entities) {
  for (int i = 0; i < entities.size(); ++i) {
    if (prev.find(entities[i]->GetId()) != prev.end()) {
      entities[i]->SetSpeed(prev[entities[i]->GetId()]);
    }
  }
  prev.clear();
}

std::string Weather::Forecast() {
  return weather.at(index);
}

bool Weather::IsCompleted() {
  return time > 60.0;
}

/*
  Weather::CreateGFX is called by SimulationModel::CreateEntity,

  Weather::CreateGFX creates our weather entity objects of type
  WeatherGFX, and stores it into our std::map that contains all
  the weather models.
*/
bool Weather::CreateGFX(JsonObject& obj, IController& CON) {
  std::string type = obj["type"];
  if (
    type.compare("hot") == 0 ||
    type.compare("snow") == 0 ||
    type.compare("rain") == 0 ||
    type.compare("tornado") == 0 ||
    type.compare("hurricane") == 0
  ) {
    GFX[type] = new WeatherGFX(obj);
    CON.AddEntity(*GFX.at(type));
    return true;
  } return false;
}

/*
  Weather::UpdateGFX is called by SimulationModel::Update, this is
  also a repeated call.

  Allows to add and remove weather models with regards to the forecast,

  Also allows to move our models if needed. Tornado movement is done
  through here.
*/
void Weather::UpdateGFX(double dt, IController& CON) {
  if (Forecast() == "snow") {
    if (restore == "none") {
      restore = "snow";
      CON.RemoveEntity(RemoveHelper(*GFX.at(restore), "AddToView"));
    }

  } else if (Forecast() == "tornado") {
    if (restore == "none") {
      restore = "tornado";
      CON.RemoveEntity(RemoveHelper(*GFX.at(restore), "AddToView"));
    }
    if (restore == "tornado") {
      GFX.at("tornado")->Update(dt, {GFX.at("tornado")});
      CON.UpdateEntity(*GFX.at("tornado"));
    }

  } else if (Forecast() == "rain") {
    if (restore == "none") {
      restore = "rain";
      CON.RemoveEntity(RemoveHelper(*GFX.at(restore), "AddToView"));
    }

  } else if (Forecast() == "hot") {
    if (restore == "none") {
      restore = "hot";
      CON.RemoveEntity(RemoveHelper(*GFX.at(restore), "AddToView"));
    }

  } else if (Forecast() == "hurricane") {
    if (time > 10.0) {
      if (restore == "none") {
        restore = "hurricane";
        CON.RemoveEntity(RemoveHelper(*GFX.at(restore), "AddToView"));
      }
    }
  }
}

/*
  Changed IController::RemoveEntity(int) --> RemoveEntity(const JsonObject&),
  Allows us to use it to either remove an entity completely or just from the view,

  Look at the transitservice.cc file, the RemoveEntity() portion of the code
*/
JsonObject Weather::RemoveHelper(const IEntity& ENT, const std::string& VU) {
  JsonObject obj;
  obj["id"] = ENT.GetId();
  obj["view"] = VU;
  JsonObject details = ENT.GetDetails();
  obj["details"] = details;
  return obj;
}

/*
  Responsible for weather model behaviors with entities.

  Weather::TornadoAct takes care of scenarios where the tornado
  makes contact with any of the entities,

  Weather::HurricaneAct removes all entities from the view for its
  period of weather.
*/
using namespace std;
void Weather::Run(const int c, IController& CON, vector<IEntity*>& ENT) {
  TornadoAct(c, CON, ENT);
  HurricaneAct(c, CON, ENT);
}

void Weather::TornadoAct(
  const int c, IController& CON, vector<IEntity*>& ENT) {
  if (Forecast() == "tornado") {
    string type = ENT[c]->GetDetails()["type"];

    if (type == "satellite" && GFX.count(type) == 0) {
      GFX[type] = ENT[c];
      JsonObject details;
      CON.RemoveEntity(RemoveHelper(*ENT[c], "RemoveFromView"));
    } else if (type == "robot") {
      string status = ENT[c]->GetDetails()["status"];
      if (status == "delete") {
        Vector3 pos(ENT[c]->GetPosition());
        Vector3 GFXPos(GFX.at("tornado")->GetPosition());
        if (GFXPos.Distance(pos) < 100) {
          string name = ENT[c]->GetDetails()["name"];
          CON.RemoveEntity(RemoveHelper(*ENT[c], "NotAssigned"));
          ENT.erase(ENT.begin() + c);
          toPublisher->SendEvents(
            "Tornado: Erased " + name + " \U0001F494\n\n");
        }
      }
    } else if (type == "human") {
      Vector3 pos(ENT[c]->GetPosition());
      Vector3 GFXPos(GFX.at("tornado")->GetPosition());
      if (GFXPos.Distance(pos) < 100) {
        string name = ENT[c]->GetDetails()["name"];
        CON.RemoveEntity(RemoveHelper(*ENT[c], "NotAssigned"));
        ENT.erase(ENT.begin() + c);
        toPublisher->SendEvents("Tornado: Erased " + name + " \U0001F494\n\n");
      }
    }
  }
}

void Weather::HurricaneAct(int c, IController& CON, vector<IEntity*>& ENT) {
  if (Forecast() == "hurricane") {
    string tag = std::to_string(ENT[c]->GetId());
    if (GFX.count(tag) == 0) {
      GFX[tag] = ENT[c];
      CON.RemoveEntity(RemoveHelper(*ENT[c], "RemoveFromView"));
    }
  }
}

/*
  Called in SimulationModel::Update

  Ensures, all the front-end changes are reveresed after weather changes.
*/
void Weather::Reverse(IController& CON, vector<IEntity*>& ENT) {
  if (restore != "none" && IsCompleted()) {
    CON.RemoveEntity(RemoveHelper(*GFX.at(restore), "RemoveFromView"));
    restore = "none";
  }

  if (Forecast() == "tornado" && IsCompleted()) {
    if (GFX.count("satellite") == 1) {
      CON.RemoveEntity(RemoveHelper(*GFX.at("satellite"), "AddToView"));
      GFX.erase("satellite");
    }
  } else if (Forecast() == "hurricane" && IsCompleted()) {
    for (int i = 0; i < ENT.size(); i++) {
      string tag = std::to_string(ENT[i]->GetId());
      if (GFX.count(tag) == 1) {
        CON.RemoveEntity(RemoveHelper(*ENT[i], "AddToView"));
        GFX.erase(tag);
      }
    }
  }
}
