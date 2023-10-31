#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include <string>
#include <vector>
#include <algorithm>

#include "ISubscriber.h"

/**
 * @brief Publisher interface
 */
class IPublisher {
 public:
  /**
   * @brief Destructor
   */
  virtual ~IPublisher() {}

  /**
   * @brief Adds the subscriber as a listener
   * @param subscriber The subscriber to add
   */
  virtual void Subscribe(ISubscriber* subscriber) = 0;

  /**
   * @brief Removes the subscriber as a listener
   * @param subscriber The subscriber to remove
   */
  virtual void UnSubscribe(ISubscriber* subscriber) = 0;

  /**
   * @brief Notifies the listeners about the current state of the object
   * @param message Contains information regarding the current state of the object
   */
  virtual void SendEvents(const std::string& message) = 0;

 protected:
  std::vector<ISubscriber*> subscribers;
};

#endif  // PUBLISHER_H_
