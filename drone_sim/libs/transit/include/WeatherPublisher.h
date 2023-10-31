#ifndef WEATHER_PUBLISHER_H_
#define WEATHER_PUBLISHER_H_

#include "IPublisher.h"

/**
 * @class WeatherPublisher
 * @brief The purpose of this class is to issue events regarding the Weather's
 *state to objects that are subscribed.
 */
class WeatherPublisher : public IPublisher {
 public:
  /**
   * @brief Constructor
   */
  WeatherPublisher() {}

  /**
   * @brief Destructor
   */
  ~WeatherPublisher() {}

  /**
   * @brief Adds the subscriber as a listener
   * @param subscriber The subscriber to add
   */
  void Subscribe(ISubscriber* subscriber);

  /**
   * @brief Removes the subscriber as a listener
   * @param subscriber The subscriber to remove
   */
  void UnSubscribe(ISubscriber* subscriber);

  /**
   * @brief Notifies the listeners about the current state of the weather
   * @param message Contains information regarding the current state of the weather
   */
  void SendEvents(const std::string& message);
};

#endif  // WEATHER_PUBLISHER_H_
