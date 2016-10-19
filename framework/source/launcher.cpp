#include "launcher.hpp"

#include <glbinding/gl/gl.h>
// load glbinding extensions
#include <glbinding/Binding.h>
// load meta info extension
#include <glbinding/Meta.h>

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "application.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"

#include <cstdlib>
#include <functional>
#include <iostream>

// use gl definitions from glbinding 
using namespace gl;

// helper functions
std::string resourcePath(int argc, char* argv[]);
void glsl_error(int error, const char* description);
void watch_gl_errors(bool activate = true);


Launcher::Launcher(int argc, char* argv[]) 
 :m_camera_fov{glm::radians(60.0f)}
 ,m_window_width{2048u}
 ,m_window_height{960u}
 ,m_window{nullptr}
 ,m_last_second_time{0.0}
 ,m_frames_per_second{0u}
 ,m_resource_path{resourcePath(argc, argv)}
 ,m_application{}
{}

std::string resourcePath(int argc, char* argv[]) {
  std::string resource_path{};
  //first argument is resource path
  if (argc > 1) {
    resource_path = argv[1];
  }
  // no resource path specified, use default
  else {
    std::string exe_path{argv[0]};
    resource_path = exe_path.substr(0, exe_path.find_last_of("/\\"));
    resource_path += "/../../resources/";
  }

  return resource_path;
}

void Launcher::initialize() {

  glfwSetErrorCallback(glsl_error);

  if (!glfwInit()) {
    std::exit(EXIT_FAILURE);
  }

  // set OGL version explicitly 
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  //MacOS requires core profile
  #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  #else
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
  #endif
  // create m_window, if unsuccessfull, quit
  m_window = glfwCreateWindow(m_window_width, m_window_height, "OpenGL Framework", NULL, NULL);
  if (!m_window) {
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  // use the windows context
  glfwMakeContextCurrent(m_window);
  // disable vsync
  glfwSwapInterval(0);
  // set user pointer to access this instance statically
  glfwSetWindowUserPointer(m_window, this);
  // register key input function
  auto key_func = [](GLFWwindow* w, int a, int b, int c, int d) {
        static_cast<Launcher*>(glfwGetWindowUserPointer(w))->key_callback(w, a, b, c, d);
  };
  glfwSetKeyCallback(m_window, key_func);
  // allow free mouse movement
  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // register resizing function
  auto resize_func = [](GLFWwindow* w, int a, int b) {
        static_cast<Launcher*>(glfwGetWindowUserPointer(w))->update_projection(w, a, b);
  };
  glfwSetFramebufferSizeCallback(m_window, resize_func);

  // initialize glindings in this context
  glbinding::Binding::initialize();

  // activate error checking after each gl function call
  watch_gl_errors();
}

const float earth_size = 1.0f;
 
void Launcher::mainLoop() {
  // do before framebuffer_resize call as it requires the projection uniform location
  // throw exception if shader compilation was unsuccessfull
  update_shader_programs(true);

  // enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  
  // rendering loop
  while (!glfwWindowShouldClose(m_window)) {
    // query input
    glfwPollEvents();
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw geometry
    std::vector<Application::planet> planets;

    //Push back new subject created with default constructor.
    planets.push_back(Application::planet());
    planets.push_back(Application::planet());
    planets.push_back(Application::planet());
    planets.push_back(Application::planet());
    planets.push_back(Application::planet());
    planets.push_back(Application::planet());
    planets.push_back(Application::planet());
    planets.push_back(Application::planet());
    planets.push_back(Application::planet());
    
    //Assign every value to simulate original distance, size, speed and rotation of planets

    //Sun
    planets[0].distance = 0.0f;
    planets[0].size = 7.0f;
    planets[0].speed = 0.0f;
    planets[0].rotation = 1.0f;

    //Mercury
    planets[1].distance = 8.0f;
    planets[1].size = earth_size * 0.7f;
    planets[1].speed = 2.5f;
    planets[1].rotation = 1.0f;

    //venus 
    planets[2].distance = 9.0f;
    planets[2].size = earth_size * 0.9f;
    planets[2].speed = 2.0f;
    planets[2].rotation = 1.0f;

    //earth
    planets[3].distance = 10.0f;
    planets[3].size = earth_size;
    planets[3].speed = 1.2f;
    planets[3].rotation = 1.0f;    

    //mars
    planets[4].distance = 11.0f;
    planets[4].size = earth_size * 0.5f;
    planets[4].speed = 1.0f;
    planets[4].rotation = 1.0f;
    //jupiter
    planets[5].distance = 15.0f;
    planets[5].size = earth_size * 1.4f;
    planets[5].speed = 0.8f;
    planets[5].rotation = 1.0f;
    //saturn
    planets[6].distance = 18.0f;
    planets[6].size = earth_size * 1.3f;
    planets[6].speed = 0.7f;
    planets[6].rotation = 1.0f;
    //uranus
    planets[7].distance = 14.0f;
    planets[7].size = earth_size * 3.0f;
    planets[7].speed = 0.5f;
    planets[7].rotation = 1.0f;
    //neptun
    planets[8].distance = 15.0f;
    planets[8].size = earth_size * 1.1f;
    planets[8].speed = 0.4f;
    planets[8].rotation = 1.0f;

    //Iterate the container which holds the sun and planets and send
    //each to upload_planet_transforms to set objects and render
    for(std::vector<Application::planet>::iterator it = planets.begin(); it != planets.end(); ++it) {
    /* std::cout << *it; ... */
      Application::planet pl = *it;
      m_application->upload_planet_transforms(pl);
    }

    
    // swap draw buffer to front
    glfwSwapBuffers(m_window);
    // display fps
    show_fps();
  }

  quit(EXIT_SUCCESS);
}

///////////////////////////// update functions ////////////////////////////////
// update viewport and field of view
void Launcher::update_projection(GLFWwindow* m_window, int width, int height) {
  // resize framebuffer
  glViewport(0, 0, width, height);

  float aspect = float(width) / float(height);
  float fov_y = m_camera_fov;
  // if width is smaller, extend vertical fov 
  if (width < height) {
    fov_y = 2.0f * glm::atan(glm::tan(m_camera_fov * 0.5f) * (1.0f / aspect));
  }
  // projection is hor+ 
  glm::fmat4 camera_projection = glm::perspective(fov_y, aspect, 0.1f, 100.0f);
  // upload matrix to gpu
  m_application->setProjection(camera_projection);
}

// load shader programs and update uniform locations
void Launcher::update_shader_programs(bool throwing) {
  // actual functionality in lambda to allow update with and without throwing
  auto update_lambda = [&](){
    // reload all shader programs
    for (auto& pair : m_application->getShaderPrograms()) {
      // throws exception when compiling was unsuccessfull
      GLuint new_program = shader_loader::program(pair.second.vertex_path,
                                                  pair.second.fragment_path);
      // free old shader program
      glDeleteProgram(pair.second.handle);
      // save new shader program
      pair.second.handle = new_program;
    }
  };

  if (throwing) {
    update_lambda();
  }
  else {
    try {
     update_lambda();
    }
    catch(std::exception&) {
      // dont crash, allow another try
    }
  }

  // after shader programs are recompiled, uniform locations may change
  m_application->uploadUniforms();
  
  // upload projection matrix to new shaders
  int width, height;
  glfwGetFramebufferSize(m_window, &width, &height);
  update_projection(m_window, width, height);
}

///////////////////////////// misc functions ////////////////////////////////
// handle key input
void Launcher::key_callback(GLFWwindow* m_window, int key, int scancode, int action, int mods) {
  if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(m_window, 1);
  }
  else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
    update_shader_programs(false);
  }
}

// calculate fps and show in m_window title
void Launcher::show_fps() {
  ++m_frames_per_second;
  double current_time = glfwGetTime();
  if (current_time - m_last_second_time >= 1.0) {
    std::string title{"OpenGL Framework - "};
    title += std::to_string(m_frames_per_second) + " fps";

    glfwSetWindowTitle(m_window, title.c_str());
    m_frames_per_second = 0;
    m_last_second_time = current_time;
  }
}

void Launcher::quit(int status) {
  // free opengl resources
  delete m_application;
  // free glfw resources
  glfwDestroyWindow(m_window);
  glfwTerminate();

  std::exit(status);
}


void glsl_error(int error, const char* description) {
  std::cerr << "GLSL Error " << error << " : "<< description << std::endl;
}

void watch_gl_errors(bool activate) {
  if(activate) {
    // add callback after each function call
    glbinding::setCallbackMaskExcept(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue, {"glGetError", "glBegin", "glVertex3f", "glColor3f"});
    glbinding::setAfterCallback(
      [](glbinding::FunctionCall const& call) {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
          // print name
          std::cerr <<  "OpenGL Error: " << call.function->name() << "(";
          // parameters
          for (unsigned i = 0; i < call.parameters.size(); ++i)
          {
            std::cerr << call.parameters[i]->asString();
            if (i < call.parameters.size() - 1)
              std::cerr << ", ";
          }
          std::cerr << ")";
          // return value
          if(call.returnValue) {
            std::cerr << " -> " << call.returnValue->asString();
          }
          // error
          std::cerr  << " - " << glbinding::Meta::getString(error) << std::endl;
          // throw exception to allow for backtrace
          throw std::runtime_error("Execution of " + std::string(call.function->name()));
          exit(EXIT_FAILURE);
        }
      }
    );
  }
  else {
    glbinding::setCallbackMask(glbinding::CallbackMask::None);
  }
}
