#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_

#include <string>


/**
 * @brief Subscriber Interface
 **/
class ISubscriber {
 public:
  /**
   * @brief Destructor
   **/
  virtual ~ISubscriber() {}

  /**
   * @brief To notify objects about events that happen to other objects
   * @param message Contains information regarding the current state of the object
   *					  		that is being listened to
   **/
  virtual void Notify(const std::string& message) = 0;
};

#endif  // SUBSCRIBER_H_
