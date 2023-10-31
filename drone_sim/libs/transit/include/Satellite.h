#ifndef SATELLITE_H_
#define SATELLITE_H_

#include "IEntity.h"
#include "IStrategy.h"

/**
 * @class Satellite
 * @brief Represents a satellite in a physical system.
 */
class Satellite : public IEntity {
 public:
  /**
   * @brief Constructor
   * @param obj JSON object containing the satellite's information
   */
  Satellite(JsonObject& obj);

  /**
   * @brief Destructor
   */
  ~Satellite();
  /**
   * @brief Gets the speed of the satellite
   * @return The speed of the satellite
   */
  float GetSpeed() const { return speed; }

  /**
   * @brief Gets the position of the satellite
   * @return The position of the satellite
   */
  Vector3 GetPosition() const { return position; }

  /**
   * @brief Gets the direction of the satellite
   * @return The direction of the satellite
   */
  Vector3 GetDirection() const { return direction; }

  /**
   * @brief Gets the destination of the satellite
   * @return The destination of the satellite
   */
  Vector3 GetDestination() const { return destination; }

  /**
   * @brief Gets the information of the satellite
   * @return The information of the satellite
   */
  JsonObject GetDetails() const { return details; }

  /**
   * @brief Updates the satellite's position
   * @param dt Delta time
   * @param scheduler Vector containing all the entities in the system
   */
  void Update(double dt, std::vector<IEntity*> scheduler);

  /**
   * @brief Sets the position of the satellite
   * @param pos_ The new position of the satellite
   */
  void SetPosition(Vector3 pos_) { position = pos_; }

  /**
   * @brief Sets the direction of the satellite
   * @param dir_ The new direction of the satellite
   */
  void SetDirection(Vector3 dir_) { direction = dir_; }

  /**
   * @brief Sets the destination of the satellite
   * @param des_ The new destination of the satellite
   */
  void SetDestination(Vector3 des_) { destination = des_; }

  /**
   * @brief Generates a new random destination for the satallite
   *                                and creates a new path to it.
   */
  void CreateNewDestination();

  /**
   * @brief Rotates the satellite
   * @param angle The angle by which the satellite should be rotated
   */
  void Rotate(double angle);

 private:
  JsonObject details;
  Vector3 position;
  Vector3 direction;
  Vector3 destination;
  float speed;
  IStrategy* toDestination = nullptr;
};

#endif
