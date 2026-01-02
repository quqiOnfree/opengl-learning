#pragma once

#include <GL/glew.h>

#include <iostream>
#include <string>

#include "window.hpp"

class Shader {
public:
  Shader(Window &window, const std::string &vertexSource,
         const std::string &fragmentSource) {
    programID_ = CreateShader(vertexSource, fragmentSource);
    if (programID_ == 0) {
      throw std::runtime_error("Failed to create shader program");
    }

    window.addGarbageCallback(
        [program = programID_]() { glDeleteProgram(program); });
  }

  ~Shader() = default;

  void use() { glUseProgram(programID_); }

  template <class Func, class... Args>
  void setUniform(std::string_view name, Func &&func, Args &&...args) {
    GLint location = glGetUniformLocation(programID_, name.data());
    std::invoke(std::forward<Func>(func), location,
                std::forward<Args>(args)...);
  }

  unsigned int getProgramID() const { return programID_; }

private:
  static unsigned int CompileShader(int type, std::string_view source) {
    unsigned int shader = glCreateShader(type);
    const char *src = source.data();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
      int length;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
      char *message = static_cast<char *>(alloca(length + 1));
      message[length] = 0;
      glGetShaderInfoLog(shader, length, &length, message);
      std::cerr << "Failed to compile "
                << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
                << " shader\n";
      std::cerr << message << '\n';
      glDeleteShader(shader);
      return 0;
    }

    return shader;
  }

  static unsigned int CreateShader(std::string_view vertexShader,
                                   std::string_view fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
  }

private:
  unsigned int programID_;
};
