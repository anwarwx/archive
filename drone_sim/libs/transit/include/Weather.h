#ifndef WEATHER_H_
#define WEATHER_H_

#include "WeatherGFX.h"
#include "IController.h"
#include "WeatherPublisher.h"
#include <random>

/**
 * @class Weather
 * @brief This class is responsible for simulating weather behavior
 *in the physical system
 *
 * Implemented via Singleton design pattern.
 */
class Weather {
 public:
  /**
   * @brief Removing the copy constructor and assignment operator
   * so the Weather instance cannot be copied
   */
  Weather(Weather const&) = delete;
  Weather& operator=(Weather const&) = delete;

  /**
   * @brief Destructor
   */
  ~Weather();

  /**
   * @brief Gets the instance of the Weather class.
   * @return A pointer to the one-time created Weather instance.
   */
  static Weather* GetInstance();

  /**
   * @brief Updates the weather cycle,
   *        applies the current weather behavior for 60 seconds
   *
   * @param dt Delta time
   * @param entities Vector containing all the entities in the system
   */
  void Update(double dt, std::vector<IEntity*>& entities);

  /**
   * @brief Represents the behavior for a "normal" weather
   */
  void Normal();

  /**
   * @brief Represents the behavior for a "heavy snow" weather
   * @param entities Vector containing all the entities in the system
   */
  void Snow(std::vector<IEntity*>& entities);

  /**
   * @brief Represents the behavior for a "tornado" weather
   * @param entities Vector containing all the entities in the system
   */
  void Tornado(std::vector<IEntity*>& entities);

  /**
   * @brief Represents the behavior for a "light rain" weather
   */
  void Rain();

  /**
   * @brief Represents the behavior for a "blazing hot" weather
   * @param entities Vector containing all the entities in the system
   */
  void Hot(std::vector<IEntity*>& entities);

  /**
   * @brief Represents the behavior for a "hurricane" weather
   * @param entities Vector containing all the entities in the system
   */
  void Hurricane(std::vector<IEntity*>& entities);

  /**
   * @brief Reverts the behavior changes of a weather
   * @param entities Vector containing all the entities in the system
   */
  void Revert(std::vector<IEntity*>& entities);

  /**
   * @brief Gets the current weather
   * @return The current weather
   */
  std::string Forecast();

  /**
   * @brief Checks if the current weather is completed by checking the time
   * @return True if complete, false if not complete
   **/
  bool IsCompleted();

  /**
   * @brief Creates all weather models
   * @param obj The JSON object containing the weather's details
   * @param CON Allows us to update the front-end view
   * @return True if created, false if not created
   **/
  bool CreateGFX(JsonObject& obj, IController& CON);

  /**
   * @brief Update the current weather's model
   * @param dt Delta time
   * @param CON Allows us to update the front-end view
   **/
  void UpdateGFX(double dt, IController& CON);

  /**
   * @brief Weathers with front-end changes get called from here
   * @param c Constant to index current entity
   * @param CON Allows us to update the front-end view
   * @param ENT Vector containing all the entities in the system
   **/
  void Run(const int c, IController& CON, std::vector<IEntity*>& ENT);

  /**
   * @brief Deletes entities in the system based on set requirements
   * @param c Constant to index current entity
   * @param CON Allows us to update the front-end view
   * @param ENT Vector containing all the entities in the system
   **/
  void TornadoAct(const int c, IController& CON, std::vector<IEntity*>& ENT);

  /**
   * @brief Removes entities from the front-end view
   * @param c Constant to index current entity
   * @param CON Allows us to update the front-end view
   * @param ENT Vector containing all the entities in the system
   **/
  void HurricaneAct(const int c, IController& CON, std::vector<IEntity*>& ENT);

  /**
   * @brief Reverses any front-end changes by the previous weather
   * @param CON Allows us to update the front-end view
   * @param ENT Vector containing all the entities in the system
   **/
  void Reverse(IController& CON, std::vector<IEntity*>& ENT);

  /**
   * @brief Gets the publisher for weather
   * @return The weather's publisher
   **/
  IPublisher* GetPublisher() { return toPublisher; }

  /**
   * @brief Helper function for RemoveEntity
   * @param ENT The entity to remove
   * @param VU The string containing information on whether
   *           removing entity from the simulation or
   *           adding/removing entity from the view 
   *
   * @return The JsonObject containing details for removing entity
   **/
  JsonObject RemoveHelper(const IEntity& ENT, const std::string& VU);

 private:
  explicit Weather() : time(0.0), index(0) {
    weather = {
      "normal", "snow", "tornado",
      "rain", "hot", "hurricane"
    };
  }
  JsonObject details;
  std::vector<std::string> weather;
  float time;
  int index;
  bool run = true;
  std::map<int, float> prev;
  std::map<std::string, IEntity*> GFX;
  std::string restore = "none";
  std::default_random_engine GEN;
  /*
    decides weather occurrence { normal, snow, tornado, rain, hot, hurricane }
    occurrence = int / length

    e.g.
      probability of tornado occurring; 3/6 times

    for testing purposes: one can set certain values to 0,
    and they won't occur, while others will
  */
  std::discrete_distribution<int> val {1, 3, 3, 1, 3, 2};
  IPublisher* toPublisher = new WeatherPublisher();
};

#endif  // WEATHER_H_
