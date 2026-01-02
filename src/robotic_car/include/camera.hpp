#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
  glm::vec3 position_;
  glm::vec3 front_;
  glm::vec3 up_;

public:
  const float velocity_constant = 25.0f;

  Camera()
      : position_(0.0f, 100.0f, 0.0f), front_(0.0f, 0.0f, -1.0f),
        up_(0.0f, 1.0f, 0.0f) {}

  ~Camera() = default;

  glm::mat4 getViewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
  }

  void moveForward(float deltaTime) {
    position_ += front_ * getVelocity(deltaTime);
  }

  void moveBackward(float deltaTime) {
    position_ -= front_ * getVelocity(deltaTime);
  }

  void moveLeft(float deltaTime) {
    position_ -=
        glm::normalize(glm::cross(front_, up_)) * getVelocity(deltaTime);
  }

  void moveRight(float deltaTime) {
    position_ +=
        glm::normalize(glm::cross(front_, up_)) * getVelocity(deltaTime);
  }

  void moveUp(float deltaTime) { position_ += up_ * getVelocity(deltaTime); }

  void moveDown(float deltaTime) { position_ -= up_ * getVelocity(deltaTime); }

  glm::vec3 getPosition() const { return position_; }
  glm::vec3 getFront() const { return front_; }
  glm::vec3 getUp() const { return up_; }

  void setPosition(const glm::vec3 &position) { position_ = position; }
  void setFront(const glm::vec3 &front) { front_ = front; }
  void setUp(const glm::vec3 &up) { up_ = up; }

private:
  float getVelocity(float deltaTime) { return velocity_constant * deltaTime; }
};
