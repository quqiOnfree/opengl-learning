#pragma once

#include <type_traits>

#include <glm/glm.hpp>

class RoboticCar
{
public:
    RoboticCar() = default;
    ~RoboticCar() = default;

    void start() {

    }

    void stop() {

    }

private:
    glm::vec3 position_;
    glm::vec3 direction_;
    
};
