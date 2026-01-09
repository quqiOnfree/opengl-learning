#pragma once

#include <stdexcept>
#include <string_view>
#include <type_traits>

#ifndef STBI_INCLUDE_STB_IMAGE_H
#include <stb_image.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "window.hpp"

class Texture {
public:
  Texture(const Window& window, std::string_view image_path)
      : shader_(R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}
)",
                                  R"(
#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}
)") {
    if (unsigned int program = shader_.getProgramID(); program == 0) {
      throw std::runtime_error("Failed to create shader program");
    }

    // load image, create texture and generate mipmaps
    int width = 0, height = 0, nrChannels = 0;
    // flip vertically so texture coordinates match image origin if needed
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data =
        stbi_load(image_path.data(), &width, &height, &nrChannels, 0);
    if (data == nullptr) {
      throw std::runtime_error("Failed to load texture image: " +
                               std::string(image_path));
    }
    std::array<float, 32> vertices = {
        // positions                          // colors           // texture
        // coords
        1.0f * static_cast<float>(width),
        1.0f * static_cast<float>(height),
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f, // top right
        1.0f * static_cast<float>(width),
        0.0f * static_cast<float>(height),
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f, // bottom right
        0.0f * static_cast<float>(width),
        0.0f * static_cast<float>(height),
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f, // bottom left
        0.0f * static_cast<float>(width),
        1.0f * static_cast<float>(height),
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f // top left
    };
    std::array<unsigned int, 6> indices = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    shader_.use();

    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    glBindVertexArray(VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
                 GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        reinterpret_cast<void *>( // NOLINT(performance-no-int-to-ptr)
            3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        reinterpret_cast<void *>( // NOLINT(performance-no-int-to-ptr)
            6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D,
                  texture_); // all upcoming GL_TEXTURE_2D operations now have
                             // effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_MIRRORED_REPEAT); // set texture wrapping to GL_REPEAT
                                         // (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = GL_RGB;
    if (nrChannels == 1)
      format = GL_RED;
    else if (nrChannels == 3)
      format = GL_RGB;
    else if (nrChannels == 4)
      format = GL_RGBA;
    // ensure byte-aligned rows for image uploads
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLint internalFormat = GL_RGB8;
    if (format == GL_RED)
      internalFormat = GL_R8;
    else if (format == GL_RGB)
      internalFormat = GL_RGB8;
    else if (format == GL_RGBA)
      internalFormat = GL_RGBA8;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    // set the sampler uniform to texture unit 0
    shader_.use();
    GLint texLoc = glGetUniformLocation(shader_.getProgramID(), "ourTexture");
    if (texLoc >= 0) {
      glUniform1i(texLoc, 0);
    }

    reloadProjection(window);
  }

  void reloadProjection(const Window &window) {
    glm::mat4 projection = glm::mat4(1.0f);
    projection =
        glm::perspective(glm::radians(45.0f),
                         static_cast<float>(window.getWidth()) * 1.0f /
                             static_cast<float>(window.getHeight()),
                         0.1f, 10000.0f);
    shader_.use();
    shader_.setUniform(
        "projection",
        [](GLint loc, const glm::mat4 &projection) {
          glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection));
        },
        projection);
  }

  void updateView(const glm::mat4 &view) {
    shader_.use();
    shader_.setUniform(
        "view",
        [](GLint loc, const glm::mat4 &view) {
          glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
        },
        view);
    glm::mat4 model = glm::mat4(1.0f);
    model =
        glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f),
    // glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

    shader_.setUniform(
        "model",
        [](GLint loc, const glm::mat4 &model) {
          glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));
        },
        model);
  }

  void draw() {
    shader_.use();
    // bind our texture to texture unit 0 before drawing
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);

    // draw our triangles
    glBindVertexArray(VAO_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  }

  ~Texture() noexcept {
    glDeleteTextures(1, &texture_);
    glDeleteVertexArrays(1, &VAO_);
    glDeleteBuffers(1, &VBO_);
    glDeleteBuffers(1, &EBO_);
  }
  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;
  Texture(Texture &&t) noexcept
      : shader_(std::move(t.shader_)), texture_(t.texture_),
        VAO_(t.VAO_), VBO_(t.VBO_), EBO_(t.EBO_) {
    t.texture_ = 0;
    t.VAO_ = 0;
    t.VBO_ = 0;
    t.EBO_ = 0;
  }
  Texture &operator=(Texture &&t) noexcept {
    if (this != &t) {
      shader_ = std::move(t.shader_);
      texture_ = t.texture_;
      VAO_ = t.VAO_;
      VBO_ = t.VBO_;
      EBO_ = t.EBO_;
      t.texture_ = 0;
      t.VAO_ = 0;
      t.VBO_ = 0;
      t.EBO_ = 0;
    }
    return *this;
  }

private:
  Shader shader_;
  unsigned int texture_ = 0;
  unsigned int VAO_ = 0;
  unsigned int VBO_ = 0;
  unsigned int EBO_ = 0;
};
