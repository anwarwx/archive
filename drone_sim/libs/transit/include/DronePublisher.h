#ifndef DRONE_PUBLISHER_H_
#define DRONE_PUBLISHER_H_

#include "IPublisher.h"

/**
 * @class DronePublisher
 * @brief The purpose of this class is to issue events regarding the Drone's
 *state to objects that are subscribed.
 */
class DronePublisher : public IPublisher {
 public:
  /**
   * @brief Constructor
   */
  DronePublisher() {}

  /**
   * @brief Destructor
   */
  ~DronePublisher() {}

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
   * @brief Notifies the listeners about the current state of the drone
   * @param message Contains information regarding the current state of the drone
   */
  void SendEvents(const std::string& message);

 private:
  std::vector<std::string> keys = {
    "Picking", "Picked",
    "Delivering", "Delivered"
  };
  std::string run = "NULL";
};

#endif  // DRONE_PUBLISHER_H_
