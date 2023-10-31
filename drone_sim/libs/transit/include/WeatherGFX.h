#ifndef WEATHER_GFX_H_
#define WEATHER_GFX_H_

#include "IEntity.h"
#include "BeelineStrategy.h"

/**
 * @class WeatherGFX
 * @brief Represents weather in the physical system
 *
 * WeatherGFX can represent snow, tornado, clouds, sun, hurricane, etc.
 *WeatherGFX is an entity, so it has the characterisitics of the IEntity class.
 *This allows us to move the physical form of weather around.
 */
class WeatherGFX : public IEntity {
 public:
  /**
   * @brief Constructor
   * @param obj The JSON object containing the weather's information
   */
  WeatherGFX(JsonObject& obj);

  /**
   * @brief Destructor
   */
  ~WeatherGFX() override = default;

  /**
   * @brief Get the weather's position
   * @return The weather's position
   */
  Vector3 GetPosition() const { return position; }

  /**
   * @brief Get the weather's direction
   * @return The the weather's direction
   */
  Vector3 GetDirection() const { return direction; }

  /**
   * @brief Get the weather's destination
   * @return The the weather's destination
   */
  Vector3 GetDestination() const { return destination; }

  /**
   * @brief Get the weather's details
   * @return The weather's details
   */
  JsonObject GetDetails() const { return details; }

  /**
   * @brief Get the weather's speed
   * @return The weather's speed
   */
  float GetSpeed() const { return speed; }

  /**
   * @brief Updates the weather's position
   * @param dt Delta time
   * @param IGNORE Filler argument for polymorphism purposes
   */
  void Update(double dt, std::vector<IEntity*> IGNORE) override;

  /**
   * @brief Sets the position of the weather
   * @param pos_ The new position of the weather
   */
  void SetPosition(Vector3 pos_) { position = pos_; }

  /**
   * @brief Sets the direction of the weather
   * @param dir_ The new direction of the weather
   */
  void SetDirection(Vector3 dir_) { direction = dir_; }

 private:
  JsonObject details;
  Vector3 position;
  Vector3 direction;
  Vector3 destination;
  float speed;
  IStrategy* toDestination = nullptr;
};

#endif  // WEATHER_GFX_H_
