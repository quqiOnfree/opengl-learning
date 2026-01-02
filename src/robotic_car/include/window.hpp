#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <string>
#include <string_view>

#include <glm/glm.hpp>

#include "camera.hpp"
#include "keyboard.hpp"

class Window {
public:
  ~Window() {
    for (auto &func : garbage_callbacks_) {
      std::invoke(func);
    }

    glfwTerminate();
  }

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(Window &&) = delete;

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }
  const std::string &getTitle() const { return title_; }

  static Window &getInstance() {
    static Window instance;
    return instance;
  }

  void initialize(int width, int height, std::string_view title) {
    width_ = width;
    height_ = height;
    title_ = title;

    /* Initialize the library */
    if (!glfwInit()) {
      throw std::runtime_error("GLFW init error");
    }

    /* Create a windowed mode window and its OpenGL context */
    window_ = glfwCreateWindow(width_, height_, title_.c_str(), NULL, NULL);
    if (!window_) {
      glfwTerminate();
      throw std::runtime_error("GLFW window creation error");
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window_);
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(
        window_, [](GLFWwindow *window, double xpos, double ypos) {
          static float lastX = xpos, lastY = ypos;
          static float yaw = -90.0f;
          static float pitch = 0.0f;

          float xoffset = xpos - lastX;
          float yoffset = lastY - ypos;
          lastX = xpos;
          lastY = ypos;

          float sensitivity = 0.05f;
          xoffset *= sensitivity;
          yoffset *= sensitivity;

          yaw += xoffset;
          pitch += yoffset;

          if (pitch > 89.0f)
            pitch = 89.0f;
          if (pitch < -89.0f)
            pitch = -89.0f;

          glm::vec3 front;
          front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
          front.y = sin(glm::radians(pitch));
          front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
          camera_.setFront(glm::normalize(front));
        });
    glEnable(GL_DEPTH_TEST);

    // enable experimental features for core contexts
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
      throw std::runtime_error("GLEW init error");
    }
  }

  template <typename Func, typename... Args,
            std::enable_if_t<
                std::is_invocable_v<Func, float, glm::mat4, Args...>, int> = 0>
  void run(Func &&func, Args &&...args) {
    float lastFrame = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window_)) {
      /* Render here */
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      float time = (float)glfwGetTime();
      float deltaTime = time - lastFrame;
      lastFrame = time;
      processKeyboardInput(window_, deltaTime, camera_);

      glm::mat4 view = camera_.getViewMatrix();
      std::invoke(std::forward<Func>(func), deltaTime, std::move(view),
                  std::forward<Args>(args)...);

      /* Swap front and back buffers */
      glfwSwapBuffers(window_);

      /* Poll for and process events */
      glfwPollEvents();
    }
  }

  GLFWwindow *getGLFWwindow() const { return window_; }
  Camera &getCamera() { return camera_; }
  const Camera &getCamera() const { return camera_; }

  template <typename Func, std::enable_if_t<std::is_invocable_v<Func>, int> = 0>
  void addGarbageCallback(Func &&func) {
    garbage_callbacks_.emplace_back(std::forward<Func>(func));
  }

private:
  Window() : width_(0), height_(0) {}

private:
  inline static Camera camera_ = {};
  GLFWwindow *window_;
  int width_;
  int height_;
  std::string title_;
  std::vector<std::function<void()>> garbage_callbacks_;
};
