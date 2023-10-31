#include "WeatherPublisher.h"

void WeatherPublisher::Subscribe(ISubscriber* subscriber) {
  subscribers.push_back(subscriber);
}

void WeatherPublisher::UnSubscribe(ISubscriber* subscriber) {
  subscribers.erase(
    std::find(subscribers.begin(), subscribers.end(), subscriber));
}

void WeatherPublisher::SendEvents(const std::string& message) {
  for (int i = 0; i < subscribers.size(); ++i) {
    subscribers[i]->Notify(message);
  }
}
