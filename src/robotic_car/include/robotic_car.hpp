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
#include "window.hpp"

class CarModel {
public:
  enum class Color : std::uint8_t { Yellow, Green, Blue };

  struct Sensor {
    glm::vec3 relative_position;
    Color color;
    float scale;
  };

  CarModel(Window &window, const MyMesh &mesh)
      : window_(window), cubeVBO_(mesh), cubeVAO_(cubeVBO_, []() {
          glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                                (void *)0);
          glEnableVertexAttribArray(0);
        }) {
    cubeVAO_.bind();
    cubeVBO_.bind();
    cubeVAO_.unbind();

    reloadProjection();
  }
  ~CarModel() = default;

  void reloadProjection() {
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(
        glm::radians(45.0f), window_.getWidth() * 1.0f / window_.getHeight(),
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
  Window &window_;
  MeshVertexBufferObject cubeVBO_;
  VertexArrayObject cubeVAO_;
  inline static constexpr char vertex_glsl[] = R"(
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
  inline static constexpr char fragment_glsl[] = R"(
#version 330 core
out vec4 FragColor;

void main()
{{
    FragColor = vec4({}, {}, {}, 1.0);
}})";
  Shader yellow_shader_ = {window_, vertex_glsl,
                           std::format(fragment_glsl, 1.0f, 1.0f, 0.0f)};
  Shader green_shader_ = {window_, vertex_glsl,
                          std::format(fragment_glsl, 0.0f, 1.0f, 0.0f)};
  Shader blue_shader_ = {window_, vertex_glsl,
                         std::format(fragment_glsl, 0.0f, 0.0f, 1.0f)};
};

class RoboticCar {
public:
  RoboticCar(Window &window, const MyMesh &mesh,
             std::string_view line_image_path)
      : window_(window),
        image_data_([line_image_path, this]() -> unsigned char * {
          stbi_set_flip_vertically_on_load(true);
          unsigned char *data = stbi_load(line_image_path.data(), &image_width_,
                                          &image_height_, &image_channels_, 0);
          if (!data) {
            throw std::runtime_error("Failed to load texture image");
          }
          return data;
        }()),
        position_({0.0f, 1.5f, 0.0f}), direction_({0.0f, 0.0f, 1.0f}),
        velocity_(10.0f), carModel_(window, mesh) {}

  ~RoboticCar() = default;

  void setPosition(const glm::vec3 &position) { position_ = position; }

  void setDirection(const glm::vec3 &direction) {
    direction_ = glm::normalize(direction);
  }

  void reloadProjection() { carModel_.reloadProjection(); }

  void updateView(const glm::mat4 &view) { carModel_.updateView(view); }

  void update(float deltaTime) {
    // Update car state based on deltaTime if needed
    auto process = [this](auto &...args) {
      std::mdspan<unsigned char, std::dextents<std::size_t, 3>> mdspan(
          image_data_.get(), image_height_, image_width_, image_channels_);
      auto is_white = [this,
                       mdspan](const glm::vec3 &relative_position) -> bool {
        float angle = glm::atan(direction_.x, direction_.z);
        glm::mat4 matrix =
            glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec4 rotated_position =
            matrix * glm::vec4(relative_position, 1.0f);
        return mdspan[static_cast<std::size_t>(
                          (position_.z + rotated_position.z) * 10.0f),
                      static_cast<std::size_t>(
                          (position_.x + rotated_position.x) * 10.0f),
                      0] > 128;
      };
      auto changeColor = [](CarModel::Sensor &sensor, CarModel::Color color) {
        sensor.color = color;
      };
      (changeColor(args, is_white(args.relative_position)
                             ? CarModel::Color::Green
                             : CarModel::Color::Blue),
       ...);
      algorithm(is_white(args.relative_position)...);
    };
    process(sensor1_, sensor2_, sensor3_, sensor4_, sensor5_);
    position_ += velocity_ * deltaTime * direction_;
  }

  void draw() {
    carModel_.draw(position_, direction_, sensor1_, sensor2_, sensor3_,
                   sensor4_, sensor5_);
  }

private:
  struct ImageDeleter {
    void operator()(unsigned char *data) const { stbi_image_free(data); }
  };

  Window &window_;
  std::unique_ptr<unsigned char, ImageDeleter> image_data_;
  int image_width_;
  int image_height_;
  int image_channels_;
  glm::vec3 position_;
  glm::vec3 direction_;
  float velocity_;
  CarModel carModel_;

  CarModel::Sensor sensor1_ = {.relative_position =
                                   glm::vec3(-3.0f, 0.0f, 2.0f),
                               .color = CarModel::Color::Blue,
                               .scale = 0.1f};
  CarModel::Sensor sensor2_ = {.relative_position =
                                   glm::vec3(-1.0f, 0.0f, 2.0f),
                               .color = CarModel::Color::Blue,
                               .scale = 0.1f};
  CarModel::Sensor sensor3_ = {.relative_position = glm::vec3(0.0f, 0.0f, 3.0f),
                               .color = CarModel::Color::Blue,
                               .scale = 0.1f};
  CarModel::Sensor sensor4_ = {.relative_position = glm::vec3(1.0f, 0.0f, 2.0f),
                               .color = CarModel::Color::Blue,
                               .scale = 0.1f};
  CarModel::Sensor sensor5_ = {.relative_position = glm::vec3(3.0f, 0.0f, 2.0f),
                               .color = CarModel::Color::Blue,
                               .scale = 0.1f};

  enum class State { Forward, TurnLeft, TurnRight, Stop };

  void algorithm(bool sensor1, bool sensor2, bool sensor3, bool sensor4,
                 bool sensor5) {
    static State current_state = State::Forward;
    if (!sensor3) {
      // 前方有线，继续前进
      current_state = State::Forward;
    } else if (sensor2 && !sensor4) {
      // 左侧有线，右侧无线，向右转
      current_state = State::TurnRight;
    } else if (!sensor2 && sensor4) {
      // 右侧有线，左侧无线，向左转
      current_state = State::TurnLeft;
    } else if (sensor1 && !sensor5) {
      // 左侧最远有线，右侧最远无线，向右转
      current_state = State::TurnRight;
    } else if (!sensor1 && sensor5) {
      // 右侧最远有线，左侧最远无线，向左转
      current_state = State::TurnLeft;
    }
    switch (current_state) {
    case State::Forward:
      // Already handled above
      break;
    case State::TurnLeft: {
      glm::vec4 curr_direction = glm::vec4(direction_, 1.0f);
      glm::mat4 rotation =
          glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f / 5.0f),
                      glm::vec3(0.0f, 1.0f, 0.0f));
      direction_ = glm::normalize(glm::vec3(rotation * curr_direction));
    } break;
    case State::TurnRight: {
      glm::vec4 curr_direction = glm::vec4(direction_, 1.0f);
      glm::mat4 rotation =
          glm::rotate(glm::mat4(1.0f), glm::radians(1.0f / 5.0f),
                      glm::vec3(0.0f, 1.0f, 0.0f));
      direction_ = glm::normalize(glm::vec3(rotation * curr_direction));
    } break;
    case State::Stop:
      velocity_ = 0.0f;
      break;
    default:
      break;
    }
  }
};
