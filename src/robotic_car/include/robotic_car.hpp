#pragma once

#include <cstdint>
#include <format>
#include <mdspan>
#include <memory>
#include <print>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef STBI_INCLUDE_STB_IMAGE_H
#include <stb_image.h>
#endif

#include "mesh_loader.hpp"
#include "shader.hpp"
#include "timer.hpp"
#include "window.hpp"

class CarModel {
public:
  enum class Color : std::uint8_t { Yellow, Green, Blue };

  struct Sensor {
    glm::vec3 relative_position;
    Color color;
    float scale;
  };

  CarModel(const Window &window, const MyMesh &mesh)
      : cubeVBO_(mesh), cubeVAO_(cubeVBO_, []() {
          glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                                (void *)0);
          glEnableVertexAttribArray(0);
        }) {
    cubeVAO_.bind();
    cubeVBO_.bind();
    cubeVAO_.unbind();

    reloadProjection(window);
  }
  ~CarModel() = default;
  CarModel(const CarModel &) = delete;
  CarModel &operator=(const CarModel &) = delete;
  CarModel(CarModel &&c) noexcept
      : cubeVBO_(std::move(c.cubeVBO_)), cubeVAO_(std::move(c.cubeVAO_)),
        yellow_shader_(std::move(c.yellow_shader_)),
        green_shader_(std::move(c.green_shader_)),
        blue_shader_(std::move(c.blue_shader_)) {}
  CarModel &operator=(CarModel &&c) noexcept {
    if (this != &c) {
      cubeVBO_ = std::move(c.cubeVBO_);
      cubeVAO_ = std::move(c.cubeVAO_);
      yellow_shader_ = std::move(c.yellow_shader_);
      green_shader_ = std::move(c.green_shader_);
      blue_shader_ = std::move(c.blue_shader_);
    }
    return *this;
  }

  void reloadProjection(const Window &window) {
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f),
                                  static_cast<float>(window.getWidth()) * 1.0f /
                                      static_cast<float>(window.getHeight()),
                                  0.1f, 10000.0f);

    yellow_shader_.use();
    yellow_shader_.setUniform(
        "projection",
        [](GLint location, const glm::mat4 &projection) {
          glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection));
        },
        projection);

    green_shader_.use();
    green_shader_.setUniform(
        "projection",
        [](GLint location, const glm::mat4 &projection) {
          glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection));
        },
        projection);

    blue_shader_.use();
    blue_shader_.setUniform(
        "projection",
        [](GLint location, const glm::mat4 &projection) {
          glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection));
        },
        projection);
  }

  void updateView(const glm::mat4 &view) {
    yellow_shader_.use();
    yellow_shader_.setUniform(
        "view",
        [](GLint loc, const glm::mat4 &view) {
          glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
        },
        view);

    green_shader_.use();
    green_shader_.setUniform(
        "view",
        [](GLint loc, const glm::mat4 &view) {
          glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
        },
        view);

    blue_shader_.use();
    blue_shader_.setUniform(
        "view",
        [](GLint loc, const glm::mat4 &view) {
          glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
        },
        view);
  }

  template <typename... Args>
    requires requires {
      (std::is_same_v<std::remove_cvref_t<Args>, Sensor> && ...);
    }
  void draw(const glm::vec3 &position, const glm::vec3 &direction,
            Args &&...args) {
    drawSingle(position, direction,
               Sensor{
                   .relative_position = glm::vec3(0.0f),
                   .color = Color::Yellow,
                   .scale = 1.0f,
               });

    (drawSingle(position, direction, std::forward<Args>(args)), ...);
  }

private:
  template <typename Params>
    requires std::is_same_v<std::remove_cvref_t<Params>, Sensor>
  void drawSingle(const glm::vec3 &position, const glm::vec3 &direction,
                  Params &&params) {
    glm::vec3 curr_postition = position;
    curr_postition.z =
        -curr_postition.z; // Invert Z axis for OpenGL coordinate system
    glm::vec3 curr_direction = glm::normalize(direction);
    std::remove_cvref_t<Params> curr_params = std::forward<Params>(params);
    curr_params.relative_position.z = -curr_params.relative_position.z;

    // 平移 -> 旋转 -> 缩放
    glm::mat4 model = glm::mat4(1.0f);
    // 先平移到世界位置
    model = glm::translate(model, curr_postition);
    // 然后旋转以面向方向
    float angle = glm::atan(-curr_direction.x, curr_direction.z);
    model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    // 再平移到相对位置
    model = glm::translate(model, curr_params.relative_position);
    // 再平移到模型中心
    model = glm::translate(model, -glm::vec3(1.5f) * curr_params.scale);
    // 最后缩放
    model = glm::scale(model, glm::vec3(0.1 * curr_params.scale,
                                        0.1 * curr_params.scale,
                                        0.1 * curr_params.scale));

    Shader *shader = nullptr;
    switch (curr_params.color) {
    case Color::Yellow:
      shader = &yellow_shader_;
      break;
    case Color::Green:
      shader = &green_shader_;
      break;
    case Color::Blue:
      shader = &blue_shader_;
      break;
    default:
      break;
    }

    if (shader) {
      shader->use();
      shader->setUniform(
          "model",
          [](GLint location, const glm::mat4 &model) {
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(model));
          },
          model);

      cubeVAO_.draw();
    }
  }

private:
  MeshVertexBufferObject cubeVBO_;
  VertexArrayObject cubeVAO_;
  inline static constexpr char // NOLINT(cppcoreguidelines-avoid-c-arrays)
      vertex_glsl[] =
          R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";
  inline static constexpr char // NOLINT(cppcoreguidelines-avoid-c-arrays)
      fragment_glsl[] =
          R"(
#version 330 core
out vec4 FragColor;

void main()
{{
    FragColor = vec4({}, {}, {}, 1.0);
}})";
  inline static std::string yellow_fragment_glsl =
      std::format(fragment_glsl, 1.0f, 1.0f, 0.0f);
  inline static std::string green_fragment_glsl =
      std::format(fragment_glsl, 0.0f, 1.0f, 0.0f);
  inline static std::string blue_fragment_glsl =
      std::format(fragment_glsl, 0.0f, 0.0f, 1.0f);
  Shader yellow_shader_ = {vertex_glsl, yellow_fragment_glsl};
  Shader green_shader_ = {vertex_glsl, green_fragment_glsl};
  Shader blue_shader_ = {vertex_glsl, blue_fragment_glsl};
};

class RoboticCar {
public:
  RoboticCar(const Window &window, const MyMesh &mesh,
             std::string_view line_image_path)
      : image_data_([line_image_path, this]() -> unsigned char * {
          stbi_set_flip_vertically_on_load(true);
          if (line_image_path.empty()) {
            throw std::runtime_error("Texture image path is empty");
          }
          unsigned char *data = stbi_load(line_image_path.data(), &image_width_,
                                          &image_height_, &image_channels_, 0);
          if (!data) {
            throw std::runtime_error("Failed to load texture image");
          }
          return data;
        }()),
        position_({0.0f, 1.5f, 0.0f}), direction_({0.0f, 0.0f, 1.0f}),
        carModel_(window, mesh) {}

  ~RoboticCar() = default;
  RoboticCar(const RoboticCar &) = delete;
  RoboticCar &operator=(const RoboticCar &) = delete;
  RoboticCar(RoboticCar &&) = delete;
  RoboticCar &operator=(RoboticCar &&) = delete;

  void setPosition(const glm::vec3 &position) { position_ = position; }

  void setDirection(const glm::vec3 &direction) {
    direction_ = glm::normalize(direction);
  }

  void reloadProjection(const Window &window) {
    carModel_.reloadProjection(window);
  }

  void updateView(const glm::mat4 &view) { carModel_.updateView(view); }

  void update(float deltaTime) {
    // Update car state based on deltaTime if needed
    auto process = [this](auto &...args) {
      std::mdspan<unsigned char, std::dextents<std::size_t, 3>> mdspan(
          image_data_.get(), image_height_, image_width_, image_channels_);
      float angle = glm::atan(direction_.x, direction_.z);
      glm::mat4 matrix =
          glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
      auto is_white = [this, mdspan,
                       &matrix](const glm::vec3 &relative_position) -> bool {
        glm::vec4 rotated_position =
            matrix * glm::vec4(relative_position, 1.0f);
        double average = 0.0f;
        for (size_t i = 0; i < 10; ++i) {
          for (size_t j = 0; j < 10; ++j) {
            average += mdspan[static_cast<std::size_t>(
                          (position_.z + rotated_position.z) * 10.0f) + i - 5,
                      static_cast<std::size_t>(
                          (position_.x + rotated_position.x) * 10.0f) + j - 5,
                      0];
          }
        }
        return average / 100.0 > 128;
      };
      auto changeColor = [](CarModel::Sensor &sensor, CarModel::Color color) {
        sensor.color = color;
      };
      (changeColor(args, is_white(args.relative_position)
                             ? CarModel::Color::Green
                             : CarModel::Color::Blue),
       ...);
      algorithm(!is_white(args.relative_position)...);
    };
    process(sensor1_, sensor2_, sensor3_, sensor4_, sensor5_, sensor6_);
    position_ += velocity_ * deltaTime * direction_;
  }

  void draw() {
    carModel_.draw(position_, direction_, sensor1_, sensor2_, sensor3_,
                   sensor4_, sensor5_, sensor6_);
  }

private:
  struct ImageDeleter {
    void operator()(unsigned char *data) const { stbi_image_free(data); }
  };

  int image_width_;
  int image_height_;
  int image_channels_;
  std::unique_ptr<unsigned char, ImageDeleter> image_data_;
  glm::vec3 position_;
  glm::vec3 direction_;
  float velocity_ = {10.0f};
  CarModel carModel_;

  CarModel::Sensor sensor1_ = {.relative_position =
                                   glm::vec3(-2.5f, 0.0f, 2.0f),
                               .color = CarModel::Color::Blue,
                               .scale = 0.1f};
  CarModel::Sensor sensor2_ = {.relative_position = glm::vec3(0.0f, 0.0f, 4.0f),
                               .color = CarModel::Color::Blue,
                               .scale = 0.1f};
  CarModel::Sensor sensor3_ = {.relative_position = glm::vec3(2.5f, 0.0f, 2.0f),
                               .color = CarModel::Color::Blue,
                               .scale = 0.1f};
  CarModel::Sensor sensor4_ = {.relative_position =
                                   glm::vec3(-1.5f, 0.0f, 2.0f),
                               .color = CarModel::Color::Blue,
                               .scale = 0.1f};
  CarModel::Sensor sensor5_ = {.relative_position = glm::vec3(0.0f, 0.0f, 2.0f),
                               .color = CarModel::Color::Blue,
                               .scale = 0.1f};
  CarModel::Sensor sensor6_ = {.relative_position = glm::vec3(1.5f, 0.0f, 2.0f),
                               .color = CarModel::Color::Blue,
                               .scale = 0.1f};

  enum class State : std::uint8_t { Forward, TurnLeft, TurnRight, Stop };

  void algorithm(bool s1, bool s2, bool s3, bool s4, bool s5, bool s6) {
    using enum State;
    static State current_state = Forward;

    static Timer timer;
    static int count = 0;

    if (timer.is_expired()) {
      if (!s2 && !s3 && s4 && s5 && !s6 && count == 0) {
        current_state = Forward;
        std::println("count = {}", count);
        count++;
        timer.expire_after(std::chrono::milliseconds(300));
      } else if (!s2 && s4 && s5 && s6 && count == 1) {
        current_state = TurnRight;
        std::println("count = {}", count);
        count++;
        timer.expire_after(std::chrono::milliseconds(300));
      } else if (s4 && s5 && s6 && count == 2) {
        current_state = TurnRight;
        std::println("count = {}", count);
        count++;
        timer.expire_after(std::chrono::milliseconds(400));
      } else if (s2 && !s3 && s4 && s5 && !s6 && count == 3) {
        current_state = TurnLeft;
        std::println("count = {}", count);
        count++;
        timer.expire_after(std::chrono::milliseconds(300));
      } else if (!s2 && s4 && s5 && s6 && count == 4) {
        current_state = TurnRight;
        std::println("count = {}", count);
        count++;
        timer.expire_after(std::chrono::milliseconds(300));
      } else if (!s2 && !s3 && s4 && s5 && !s6 && count == 5) {
        current_state = TurnLeft;
        std::println("count = {}", count);
        count++;
        timer.expire_after(std::chrono::milliseconds(300));
      } else if (s2 && !s3 && s4 && s5 && !s6 && count == 6) {
        current_state = Forward;
        std::println("count = {}", count);
        count++;
        timer.expire_after(std::chrono::milliseconds(300));
      } else if (s2 && s4 && s5 && s6 && count == 7) {
        current_state = TurnRight;
        std::println("count = {}", count);
        count++;
        timer.expire_after(std::chrono::milliseconds(300));
      } else if (!s1 && s2 && !s4 && s5 && s6 && count == 8) {
        current_state = TurnRight;
        std::println("count = {}", count);
        count++;
        timer.expire_after(std::chrono::milliseconds(300));
      } else if (s1 || s4) {
        current_state = TurnLeft;
      } else if (s3 || s6) {
        current_state = TurnRight;
      } else if (s2 || s5) {
        current_state = Forward;
      } else if (s1 && s2 && s3 && s4 && s5 && s6) {
        current_state = Stop;
      }
    }

    switch (current_state) {
    case Forward:
      // Already handled above
      break;
    case TurnLeft: {
      glm::vec4 curr_direction = glm::vec4(direction_, 1.0f);
      glm::mat4 rotation =
          glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f / 5.0f),
                      glm::vec3(0.0f, 1.0f, 0.0f));
      direction_ = glm::normalize(glm::vec3(rotation * curr_direction));
    } break;
    case TurnRight: {
      glm::vec4 curr_direction = glm::vec4(direction_, 1.0f);
      glm::mat4 rotation =
          glm::rotate(glm::mat4(1.0f), glm::radians(1.0f / 5.0f),
                      glm::vec3(0.0f, 1.0f, 0.0f));
      direction_ = glm::normalize(glm::vec3(rotation * curr_direction));
    } break;
    case Stop:
      velocity_ = 0.0f;
      break;
    default:
      break;
    }
  }
};
