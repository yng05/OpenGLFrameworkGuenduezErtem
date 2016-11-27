#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"
#include "pixel_data.hpp"

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

struct texture_obj {
  texture_obj (pixel_data const& t)
   : tex(t)
  {}

  GLenum context = GL_TEXTURE0;
  GLenum target = GL_TEXTURE_2D;
  GLuint tex_obj = 0;
  pixel_data tex;
};


struct planet
{
  float distance;
  float speed;
  float size; 
  float rotation;  
  std::string name;
  glm::vec3 color; 
  int order;
};

int number_of_stars;
std::vector<struct planet> planets;
std::vector<struct texture_obj> planet_textures;
std::vector<struct texture_obj> other_textures;

const float earth_size = 1.0f;

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,m_obj_planet{},m_obj_star{},m_obj_skydome{}
{  
  initializePlanets();
  initializeSkydome();
  initializeStars();
  initializeShaderPrograms();
}

void ApplicationSolar::upload_planet_transforms(struct planet pl) const {
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

  glUniform3f(m_shaders.at("planet").u_locs.at("ColorVec"), pl.color[0], pl.color[1], pl.color[2]);

  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(planet_textures[pl.order].target,planet_textures[pl.order].tex_obj);
  glUniform1i(m_shaders.at("planet").u_locs.at("Texture"), 0);


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

    glUniform3f(m_shaders.at("planet").u_locs.at("ColorVec"), 1.0f, 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(other_textures[0].target,other_textures[0].tex_obj);
    glUniform1i(m_shaders.at("planet").u_locs.at("Texture"), 0);

    // bind the VAO to draw
    glBindVertexArray(m_obj_planet.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(m_obj_planet.draw_mode, m_obj_planet.num_elements, model::INDEX.type, NULL);
  }
}

void ApplicationSolar::render() const {   
    renderSkydome();
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

void ApplicationSolar::renderSkydome() const {   
    glUseProgram(m_shaders.at("skydome").handle);

    glm::fmat4 size = glm::scale(glm::mat4{}, glm::vec3{60.0f}); 
    glm::fmat4 model_matrix = glm::rotate(size, 0.0f , glm::fvec3{0.0f, 0.1f, 0.0f});
    model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, 0.0f});
    glUniformMatrix4fv(m_shaders.at("skydome").u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));

    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
    glUniformMatrix4fv(m_shaders.at("skydome").u_locs.at("NormalMatrix"),
                       1, GL_FALSE, glm::value_ptr(normal_matrix));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(other_textures[1].target,other_textures[1].tex_obj);
    glUniform1i(m_shaders.at("skydome").u_locs.at("Texture"), 0);

    // bind the VAO to draw
    glBindVertexArray(m_obj_skydome.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(m_obj_skydome.draw_mode, m_obj_skydome.num_elements, model::INDEX.type, NULL);

  
}

void ApplicationSolar::renderStars() const {

  glUseProgram(m_shaders.at("stars").handle); 
  // bind the VAO to draw
  glBindVertexArray(m_obj_star.vertex_AO);

  glDrawArrays(GL_POINTS, 0, number_of_stars);
  
}

void ApplicationSolar::updateView() {
  // vertices are transformed in camera space, so camera transform must be inverted

  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
      // upload matrix to gpu
  glUseProgram(m_shaders.at("skydome").handle);  
  glUniformMatrix4fv(m_shaders.at("skydome").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
  // upload matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);  
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("stars").handle);
  glUniformMatrix4fv(m_shaders.at("stars").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

}

void ApplicationSolar::updateProjection() {
  // upload matrix to gpu
  glUseProgram(m_shaders.at("skydome").handle); 
  glUniformMatrix4fv(m_shaders.at("skydome").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
  glUseProgram(m_shaders.at("planet").handle); 
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
  glUseProgram(m_shaders.at("stars").handle);
  glUniformMatrix4fv(m_shaders.at("stars").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}



// update uniform locations
void ApplicationSolar::uploadUniforms() {
  updateUniformLocations();  
  
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

  m_shaders.emplace("skydome", shader_program{m_resource_path + "shaders/skydome.vert",
                                           m_resource_path + "shaders/skydome.frag"});
  // request uniform locations for shader program
  m_shaders.at("skydome").u_locs["NormalMatrix"] = -1;
  m_shaders.at("skydome").u_locs["ModelMatrix"] = -1;
  m_shaders.at("skydome").u_locs["ViewMatrix"] = -1;
  m_shaders.at("skydome").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("skydome").u_locs["Texture"] = -1;
    // store shader program objects in container
  m_shaders.emplace("stars", shader_program{m_resource_path + "shaders/stars.vert",
                                           m_resource_path + "shaders/stars.frag"});
  // request uniform locations for shader program
  
  m_shaders.at("stars").u_locs["ViewMatrix"] = 0;
  m_shaders.at("stars").u_locs["ProjectionMatrix"] = 0;

  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{m_resource_path + "shaders/simple.vert",
                                           m_resource_path + "shaders/simple.frag"});
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet").u_locs["ColorVec"] = -1;
  m_shaders.at("planet").u_locs["Texture"] = -1;

}

void ApplicationSolar::initializePlanets() {
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

    //planet_textures.push_back(texture const& texture(texture_loader::file(m_resource_path + "textures/sun.png")));
    
    //Assign every value to simulate original distance, size, speed, color and rotation of planets

    //Sun
    planets[0].distance = 0.0f;
    planets[0].size = 7.0f;
    planets[0].speed = 0.0f;
    planets[0].rotation = 1.0f;
    planets[0].name = "sun";
    planets[0].color = {0.96f,1.0f,0.0f};
    planets[0].order = 0;

    //Mercury
    planets[1].distance = 8.0f;
    planets[1].size = earth_size * 0.7f;
    planets[1].speed = 2.5f;
    planets[1].rotation = 1.0f;
    planets[1].name = "mercury";
    planets[1].color = {0.86f,0.64f,0.80f};
    planets[1].order = 1;

    //venus 
    planets[2].distance = 9.0f;
    planets[2].size = earth_size * 0.9f;
    planets[2].speed = 2.0f;
    planets[2].rotation = 1.0f;
    planets[2].name = "venus";
    planets[2].color = {0.88f,0.85f,0.45f};
    planets[2].order = 2;

    //earth
    planets[3].distance = 10.0f;
    planets[3].size = earth_size;
    planets[3].speed = 1.2f;
    planets[3].rotation = 1.0f;  
    planets[3].name = "earth";  
    planets[3].color = {0.0f,0.22f,1.0f};
    planets[3].order = 3;

    //mars
    planets[4].distance = 11.0f;
    planets[4].size = earth_size * 0.5f;
    planets[4].speed = 1.0f;
    planets[4].rotation = 1.0f;
    planets[4].name = "mars";
    planets[4].color = {0.60f,0.70f,0.80f};
    planets[4].order = 4;
    //jupiter
    planets[5].distance = 15.0f;
    planets[5].size = earth_size * 1.4f;
    planets[5].speed = 0.8f;
    planets[5].rotation = 1.0f;
    planets[5].name = "jupiter";
    planets[5].color = {0.69f,0.69f,0.69f};
    planets[5].order = 5;
    //satur
    planets[6].distance = 18.0f;
    planets[6].size = earth_size * 1.3f;
    planets[6].speed = 0.7f;
    planets[6].rotation = 1.0f;
    planets[6].name = "saturn";
    planets[6].color = {0.90f,0.13f,0.26f};
    planets[6].order = 6;
    //uranus
    planets[7].distance = 14.0f;
    planets[7].size = earth_size * 3.0f;
    planets[7].speed = 0.5f;
    planets[7].rotation = 1.0f;
    planets[7].name = "uranus";
    planets[7].color = {0.65f,0.12f,0.56f};
    planets[7].order = 7;
    //neptu
    planets[8].distance = 15.0f;
    planets[8].size = earth_size * 1.1f;
    planets[8].speed = 0.4f;
    planets[8].rotation = 1.0f;
    planets[8].name = "neptune";
    planets[8].color = {0.24f,0.48f,0.80f};
    planets[8].order = 8;

    for (auto planet: planets) {
        planet_textures.push_back(texture_loader::file(m_resource_path + "textures/" + planet.name + ".png"));
    }

    for (auto planet: planets){
      glActiveTexture(GL_TEXTURE0);

      // generate a new texture object
      glGenTextures(1, &planet_textures[planet.order].tex_obj);

      // bind the texture to the current context and target
      glBindTexture(planet_textures[planet.order].target, planet_textures[planet.order].tex_obj);

      // set texture sampling parameters
      glTexParameteri(planet_textures[planet.order].target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
      glTexParameteri(planet_textures[planet.order].target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));

      glTexImage2D(planet_textures[planet.order].target,
        0, // mipmaps
        GLint(GL_RGBA),
        planet_textures[planet.order].tex.width, planet_textures[planet.order].tex.height,
        0, // no border
        GL_RGBA,
        planet_textures[planet.order].tex.channel_type,
        planet_textures[planet.order].tex.pixels.data()
      );
    }

    other_textures.push_back(texture_loader::file(m_resource_path + "textures/moon.png")); 

    glActiveTexture(GL_TEXTURE0);

      // generate a new texture object
      glGenTextures(1, &other_textures[0].tex_obj);

      // bind the texture to the current context and target
      glBindTexture(other_textures[0].target, other_textures[0].tex_obj);

      // set texture sampling parameters
      glTexParameteri(other_textures[0].target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
      glTexParameteri(other_textures[0].target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));

      glTexImage2D(other_textures[0].target,
        0, // mipmaps
        GLint(GL_RGBA),
        other_textures[0].tex.width, other_textures[0].tex.height,
        0, // no border
        GL_RGBA,
        other_textures[0].tex.channel_type,
        other_textures[0].tex.pixels.data()
      ); 


    model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL | model::TEXCOORD);
    
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
    // activate second attribute on gpu
    glEnableVertexAttribArray(2);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(2, model::TEXCOORD.components, (gl::GLenum) model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);
 
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
void ApplicationSolar::initializeSkydome() {
  
    other_textures.push_back(texture_loader::file(m_resource_path + "textures/skydome.png"));  

    glActiveTexture(GL_TEXTURE0);

      // generate a new texture object
      glGenTextures(1, &other_textures[1].tex_obj);

      // bind the texture to the current context and target
      glBindTexture(other_textures[1].target, other_textures[1].tex_obj);

      // set texture sampling parameters
      glTexParameteri(other_textures[1].target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
      glTexParameteri(other_textures[1].target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));

      glTexImage2D(other_textures[1].target,
        0, // mipmaps
        GLint(GL_RGBA),
        other_textures[1].tex.width, other_textures[1].tex.height,
        0, // no border
        GL_RGBA,
        other_textures[1].tex.channel_type,
        other_textures[1].tex.pixels.data()
      );  

    model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL | model::TEXCOORD);

    // generate vertex array object
    glGenVertexArrays(1, &m_obj_skydome.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(m_obj_skydome.vertex_AO);

    // generate generic buffer
    glGenBuffers(1, &m_obj_skydome.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, m_obj_skydome.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
    // activate second attribute on gpu
    glEnableVertexAttribArray(1);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);
    // activate second attribute on gpu
    glEnableVertexAttribArray(2);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(2, model::TEXCOORD.components, (gl::GLenum) model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);

     // generate generic buffer
    glGenBuffers(1, &m_obj_skydome.element_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_obj_skydome.element_BO);
    // configure currently bound array buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);

    // store type of primitive to draw
    m_obj_skydome.draw_mode = GL_TRIANGLES;
    // transfer number of indices to model object 
    m_obj_skydome.num_elements = GLsizei(planet_model.indices.size());
}

void ApplicationSolar::initializeStars() {
  std::vector<float> stars;
  number_of_stars = int(fmod(std::rand() , 200));

  //Store position and color values
  for (int i=0;i<number_of_stars;i++) {
    float x = float(fmod(std::rand() ,  50)) - float(fmod(std::rand() , 75));
    float y = float(fmod(std::rand() ,  50)) - float(fmod(std::rand() , 75));
    float z = -float(fmod(std::rand() , 50));

    stars.push_back(x);
    stars.push_back(y);
    stars.push_back(z);
    // Color
    stars.push_back(1.0f);
    stars.push_back(1.0f);
    stars.push_back(1.0f);
  }
  //Load stored values in a model object and write those in buffer
  model star_model = {stars, model::POSITION | model::NORMAL};
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

