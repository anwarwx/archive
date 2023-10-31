#include "DronePublisher.h"

void DronePublisher::Subscribe(ISubscriber* subscriber) {
  subscribers.push_back(subscriber);
}

void DronePublisher::UnSubscribe(ISubscriber* subscriber) {
  subscribers.erase(
    std::find(subscribers.begin(), subscribers.end(), subscriber));
}

/*
  SendEvents gets called via Drone::Update() so it is a repeated call,
  the if-statements makes sure the message gets printed once onto the
  front-end
*/
void DronePublisher::SendEvents(const std::string& message) {
  for (int i = 0; i < subscribers.size(); ++i) {
    if (message.find(keys.at(0)) != std::string::npos) {
      if (run != keys[0]) {
        subscribers[i]->Notify(message);
        run = keys[0];
      }
    } else if (message.find(keys.at(1)) != std::string::npos) {
      if (run != keys[1]) {
        subscribers[i]->Notify(message);
        run = keys[1];
      }
    } else if (message.find(keys.at(2)) != std::string::npos) {
      if (run != keys[2]) {
        subscribers[i]->Notify(message);
        run = keys[2];
      }
    } else if (message.find(keys.at(3)) != std::string::npos) {
      if (run != keys[3]) {
        subscribers[i]->Notify(message);
        run = keys[3];
      }
    }
  }
}
