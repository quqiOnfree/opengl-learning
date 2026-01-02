#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "robotic_car.hpp"
#include "texture.hpp"
#include "window.hpp"

constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

int main() {
  Window &window = Window::getInstance();
  window.initialize(SCR_WIDTH, SCR_HEIGHT, "Robotic Car Simulation");

  Texture texture(window, "line.jpg");
  MyMesh mesh;
  {
    std::string filename = "cube.stl";
    if (!OpenMesh::IO::read_mesh(mesh, filename)) {
      std::cerr << "Error: Cannot read mesh from " << filename << '\n';
      return 0;
    }
  }
  RoboticCar car(window, mesh, "line.jpg");
  car.setPosition({16.5f, 1.51f, 20.0f});
  glm::vec3 direction = {0.0f, 0.0f, 0.5f};
  car.setDirection(direction);

  window.run([&](float deltaTime, glm::mat4 view) {
    texture.updateView(view);
    texture.draw();
    car.update(deltaTime);
    car.updateView(view);
    car.draw();
  });

  return 0;
}
