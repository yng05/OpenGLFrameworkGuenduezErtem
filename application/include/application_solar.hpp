#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"

// gpu representation of model
class ApplicationSolar : public Application {
 public:
  // allocate and initialize objects
  ApplicationSolar(std::string const& resource_path);
  // free allocated objects
  ~ApplicationSolar();

  // update uniform locations and values
  void uploadUniforms();
  // update projection matrix
  void updateProjection();
  void updateProjectionStars();
  // react to key input
  void keyCallback(int key, int scancode, int action, int mods);
  // draw all objects
  void upload_planet_transforms(struct planet pl) const;
  void render() const;
  void renderPlanets() const;
  void renderStars() const;

 protected:
  void initializeShaderPrograms();
  void initializeGeometry(model & model_);
  void initializePlanets();
  void initializeStars();
  void updateView();
  void updateViewStars();

  // cpu representation of model
  model_object m_obj_planet;
  model_object m_obj_star;
};

#endif