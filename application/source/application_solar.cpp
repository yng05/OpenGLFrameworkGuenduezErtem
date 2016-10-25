#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
    // draw all objects
struct planet
{
  float distance;
  float speed;
  float size; 
  float rotation;  
  std::string name;  
};

int number_of_stars;
std::vector<struct planet> planets;

const float earth_size = 1.0f;

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,m_obj_planet{},m_obj_star{}
{ 
  initializeStars();
  initializePlanets();
  initializeShaderPrograms();
}

void ApplicationSolar::upload_planet_transforms(struct planet pl) const {  
  //std::cout << pl.name + ":" + std::to_string(pl.size) + "---" + std::to_string(pl.speed)+ "---" + std::to_string(pl.distance)+ "---"+ std::to_string(pl.rotation) + "\n";
  
  // bind shader to upload uniforms
  glUseProgram(m_shaders.at("planet").handle); 

  //Calculate model and normal matrices and render
  glm::fmat4 size = glm::scale(glm::mat4{}, glm::vec3{pl.size}); 
  glm::fmat4 model_matrix = glm::rotate(size, float(glfwGetTime()) * pl.speed, glm::fvec3{0.0f, pl.rotation, 0.0f});  
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -pl.distance});
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);

  
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface
  
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));



  // bind the VAO to draw
  glBindVertexArray(m_obj_planet.vertex_AO);

  // draw bound vertex array using bound shader
  glDrawElements(m_obj_planet.draw_mode, m_obj_planet.num_elements, model::INDEX.type, NULL);

  //Create moon for earth distinguishing by size
  if(pl.size == 1.0f)
  {

    glm::mat4 MoonSize = glm::scale(model_matrix, glm::vec3{ 0.2f });
    glm::mat4 model_matrix = glm::rotate(MoonSize, float(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation
    model_matrix = glm::translate(model_matrix, glm::vec3{ 0.0f, 0.0f, -8.0f }); // radius length
    glm::mat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);

    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface
  
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));

    // bind the VAO to draw
    glBindVertexArray(m_obj_planet.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(m_obj_planet.draw_mode, m_obj_planet.num_elements, model::INDEX.type, NULL);
  }
}
void ApplicationSolar::render() const {   
    renderStars();  
    renderPlanets();
}

void ApplicationSolar::renderPlanets() const {   
    //Iterate the container which holds the sun and planets and send
    //each to upload_planet_transforms to set objects and render
    for(std::vector<struct planet>::iterator it = planets.begin(); it != planets.end(); ++it) {    
      struct planet pl = *it;
      upload_planet_transforms(pl);      
    }
  
}

void ApplicationSolar::renderStars() const {

  glUseProgram(m_shaders.at("stars").handle); 

  // bind the VAO to draw
  glBindVertexArray(m_obj_star.vertex_AO);

  // draw bound vertex array using bound shader
  
  utils::validate_program(m_shaders.at("stars").handle);
  glDrawArrays(GL_POINTS, 0, number_of_stars);
  
}

void ApplicationSolar::updateView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUniformMatrix4fv(m_shaders.at("stars").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

}

void ApplicationSolar::updateProjection() {
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
  glUniformMatrix4fv(m_shaders.at("stars").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}



// update uniform locations
void ApplicationSolar::uploadUniforms() {
  updateUniformLocations();


  glUseProgram(m_shaders.at("stars").handle);
  glUseProgram(m_shaders.at("planet").handle);  
  
  updateView();
  updateProjection();
}

// handle key input
void ApplicationSolar::keyCallback(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.1f});
    updateView();
  }
  else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.1f});
    updateView();
  }
}

// load shader programs
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{m_resource_path + "shaders/simple.vert",
                                           m_resource_path + "shaders/simple.frag"});
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;


  // store shader program objects in container
  m_shaders.emplace("stars", shader_program{m_resource_path + "shaders/stars.vert",
                                           m_resource_path + "shaders/stars.frag"});
  // request uniform locations for shader program
  
  m_shaders.at("stars").u_locs["ViewMatrix"] = -1;
  m_shaders.at("stars").u_locs["ProjectionMatrix"] = -1;

}

void ApplicationSolar::initializePlanets() {
  std::cout << "initializePlanets\n";
  //Push back new subject created with default constructor.
    planets.push_back(planet());
    planets.push_back(planet());
    planets.push_back(planet());
    planets.push_back(planet());
    planets.push_back(planet());
    planets.push_back(planet());
    planets.push_back(planet());
    planets.push_back(planet());
    planets.push_back(planet());
    
    //Assign every value to simulate original distance, size, speed and rotation of planets

    //Sun
    planets[0].distance = 0.0f;
    planets[0].size = 7.0f;
    planets[0].speed = 0.0f;
    planets[0].rotation = 1.0f;
    planets[0].name = "Sun";

    //Mercury
    planets[1].distance = 8.0f;
    planets[1].size = earth_size * 0.7f;
    planets[1].speed = 2.5f;
    planets[1].rotation = 1.0f;
    planets[1].name = "Mercury";

    //venus 
    planets[2].distance = 9.0f;
    planets[2].size = earth_size * 0.9f;
    planets[2].speed = 2.0f;
    planets[2].rotation = 1.0f;
    planets[2].name = "venus";

    //earth
    planets[3].distance = 10.0f;
    planets[3].size = earth_size;
    planets[3].speed = 1.2f;
    planets[3].rotation = 1.0f;  
    planets[3].name = "earth";  

    //mars
    planets[4].distance = 11.0f;
    planets[4].size = earth_size * 0.5f;
    planets[4].speed = 1.0f;
    planets[4].rotation = 1.0f;
    planets[4].name = "mars";
    //jupiter
    planets[5].distance = 15.0f;
    planets[5].size = earth_size * 1.4f;
    planets[5].speed = 0.8f;
    planets[5].rotation = 1.0f;
    planets[5].name = "jupiter";
    //saturn
    planets[6].distance = 18.0f;
    planets[6].size = earth_size * 1.3f;
    planets[6].speed = 0.7f;
    planets[6].rotation = 1.0f;
    planets[6].name = "saturn";
    //uranus
    planets[7].distance = 14.0f;
    planets[7].size = earth_size * 3.0f;
    planets[7].speed = 0.5f;
    planets[7].rotation = 1.0f;
    planets[7].name = "uranus";
    //neptun
    planets[8].distance = 15.0f;
    planets[8].size = earth_size * 1.1f;
    planets[8].speed = 0.4f;
    planets[8].rotation = 1.0f;
    planets[8].name = "neptun";

    model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);
    
    // generate vertex array object
    glGenVertexArrays(1, &m_obj_planet.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(m_obj_planet.vertex_AO);

    // generate generic buffer
    glGenBuffers(1, &m_obj_planet.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, m_obj_planet.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, (gl::GLenum) model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
    // activate second attribute on gpu
    glEnableVertexAttribArray(1);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(1, model::NORMAL.components, (gl::GLenum) model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);

     // generate generic buffer
    glGenBuffers(1, &m_obj_planet.element_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_obj_planet.element_BO);
    // configure currently bound array buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);


      // store type of primitive to draw
    m_obj_planet.draw_mode = GL_TRIANGLES;
    // transfer number of indices to model object 
    m_obj_planet.num_elements = GLsizei(planet_model.indices.size()); 
}

void ApplicationSolar::initializeStars() {
  std::cout << "initializeStars\n";
  std::vector<float> stars;
  number_of_stars = int(fmod(std::rand() , 50));
  std::cout << std::to_string(number_of_stars) + "\n";

  for (int i=0;i<number_of_stars;i++) {
    float x = float(fmod(std::rand() , 25));
    float y = float(fmod(std::rand() , 25));
    float z = float(fmod(std::rand() , 25));

    stars.push_back(x);
    stars.push_back(y);
    stars.push_back(z);
    // Color
    stars.push_back(1.0f);
    stars.push_back(1.0f);
    stars.push_back(1.0f);
  }

  model star_model = {stars, model::POSITION | model::NORMAL};
  std::cout << std::to_string(star_model.data.size()) + "\n";
// generate vertex array object
  glGenVertexArrays(1, &m_obj_star.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(m_obj_star.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &m_obj_star.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, m_obj_star.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * star_model.data.size(), star_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, (gl::GLenum) model::POSITION.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, (gl::GLenum) model::NORMAL.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets[model::NORMAL]);

   // generate generic buffer
  glGenBuffers(1, &m_obj_star.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_obj_star.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * star_model.indices.size(), star_model.indices.data(), GL_STATIC_DRAW);



    

      // store type of primitive to draw
    m_obj_star.draw_mode = GL_POINTS;
    // transfer number of indices to model object 
    m_obj_star.num_elements = GLsizei(star_model.indices.size()); 
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &m_obj_planet.vertex_BO);
  glDeleteBuffers(1, &m_obj_planet.element_BO);
  glDeleteVertexArrays(1, &m_obj_planet.vertex_AO);
  glDeleteBuffers(1, &m_obj_star.vertex_BO);
  glDeleteBuffers(1, &m_obj_star.element_BO);
  glDeleteVertexArrays(1, &m_obj_star.vertex_AO);
}

// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<ApplicationSolar>(argc, argv);
}

