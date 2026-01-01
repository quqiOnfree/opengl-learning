#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

#include "window.hpp"
#include "texture.hpp"

constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

int main(void)
{
    Window& window = Window::getInstance();
    window.initialize(SCR_WIDTH, SCR_HEIGHT, "Robotic Car Simulation");

    Texture texture(window, "line.jpg");
    
    window.run(
        [&](float deltaTime, glm::mat4 view) {
            texture.updateView(view);
            texture.draw();
        }
    );

    return 0;
}
