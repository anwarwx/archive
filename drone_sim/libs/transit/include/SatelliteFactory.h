#ifndef SATELLITE_FACTORY_H_
#define SATELLITE_FACTORY_H_

#include <vector>

#include "IEntityFactory.h"
#include "Satellite.h"

/**
 *@brief Satellite Factory to produce Satellite's class.
 **/
class SatelliteFactory : public IEntityFactory {
 public:
  /**
   * @brief Destructor for IEntityFactory class.
   **/
  virtual ~SatelliteFactory() {}

  /**
   * @brief Creates entity using the given JSON object, if possible.
   * @param entity - JsonObject to be used to create the new entity.
   * @return Entity that was created if it was created successfully, or a
   *nullpointer if creation failed.
   **/
  IEntity* CreateEntity(JsonObject& entity);
};

#endif
