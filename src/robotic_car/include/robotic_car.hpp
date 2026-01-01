#pragma once

#include <type_traits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh_loader.hpp"
#include "shader.hpp"
#include "window.hpp"

class RoboticCar
{
public:
    RoboticCar(Window& window, const MyMesh& mesh, const glm::vec3& color = {0.5f, 0.5f, 0.0f}) :
        window_(window),
        direction_({0.0f, 0.0f, -1.0f}),
        velocity_(5.0f),
        carBodyVBO_(mesh),
        carBodyVAO_(carBodyVBO_, [](){
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
            }),
        shader_(R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)", R"(
#version 330 core
out vec4 FragColor;

uniform vec3 objectColor;

void main()
{
    FragColor = vec4(objectColor, 1.0);
})")
    {
        shader_.use();
        carBodyVAO_.bind();
        carBodyVBO_.bind();
        carBodyVAO_.unbind();

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), window_.getWidth() * 1.0f / window_.getHeight(), 0.1f, 10000.0f);
        shader_.setUniform("projection", [](GLint location, const glm::mat4& projection) {
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection));
        }, projection);

        shader_.setUniform("objectColor", [](GLint location, const glm::vec3& color){
            glUniform3fv(location, 1, glm::value_ptr(color));
        }, color);
    }
    
    ~RoboticCar() = default;

    void setPosition(const glm::vec3& position) {
        position_ = position;
    }

    void setDirection(const glm::vec3& direction) {
        direction_ = direction;
    }

    void reloadProjection() {
        shader_.use();
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), window_.getWidth() * 1.0f / window_.getHeight(), 0.1f, 10000.0f);
        shader_.setUniform("projection", [](GLint location, const glm::mat4& projection) {
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection));
        }, projection);
    }

    void updateView(const glm::mat4& view) {
        shader_.use();
        shader_.setUniform("view", [](GLint loc, const glm::mat4& view){
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
        }, view);
    }

    void update(float deltaTime) {
        // Update car state based on deltaTime if needed
        position_ += velocity_ * deltaTime * direction_;
    }

    void draw() {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position_);
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        // Additional transformations based on direction_ can be added here

        // Set the model matrix uniform in the shader (not shown here)
        shader_.use();
        shader_.setUniform("model", [](GLint location, const glm::mat4& model) {
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(model));
        }, model);

        carBodyVAO_.draw();
    }

private:
    Window& window_;
    glm::vec3 position_;
    glm::vec3 direction_;
    float velocity_;
    MeshVertexBufferObject carBodyVBO_;
    VertexArrayObject carBodyVAO_;
    Shader shader_;
};
