#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <freetype/freetype.h>
#include <glm/glm.hpp>

class Font {
public:
  Font() {
    if (FT_Init_FreeType(&ft_)) {
      throw std::runtime_error("Could not init FreeType Library");
    }
  }

  ~Font() {
    if (face_) {
      FT_Done_Face(face_);
    }
    if (ft_) {
      FT_Done_FreeType(ft_);
    }
  }
  Font(const Font &) = delete;
  Font &operator=(const Font &) = delete;
  Font(Font &&) noexcept {
    std::swap(ft_, ft_);
    std::swap(face_, face_);
  }
  Font &operator=(Font &&other) noexcept {
    if (this != &other) {
      std::swap(ft_, other.ft_);
      std::swap(face_, other.face_);
    }
    return *this;
  }

  void loadFont(const std::string &font_path) {
    if (FT_New_Face(ft_, font_path.c_str(), 0, &face_)) {
      throw std::runtime_error("Failed to load font: " + font_path);
    }
  }

  FT_Face getFace() const { return face_; }

  void setSize(int width, int height) {
    FT_Set_Pixel_Sizes(face_, width, height);
  }

  void updateFont() { loadFontMap(); }

private:
  struct Character {
    GLuint TextureID;   // 字形纹理的ID
    glm::ivec2 Size;    // 字形大小
    glm::ivec2 Bearing; // 从基准线到字形左部/顶部的偏移值
    GLuint Advance;     // 原点距下一个字形原点的距离
  };

  FT_Library ft_ = nullptr;
  FT_Face face_ = nullptr;

  std::map<GLchar, Character> characters_;

  void loadFontMap() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // 禁用字节对齐限制

    for (unsigned char c = 0; c < 128; c++) {
      // 加载字符的字形
      if (FT_Load_Char(face_, c, FT_LOAD_RENDER)) {
        throw std::runtime_error("Failed to load Glyph");
      }
      // 生成纹理
      GLuint texture = {};
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                   static_cast<int>(face_->glyph->bitmap.width),
                   static_cast<int>(face_->glyph->bitmap.rows), 0, GL_RED,
                   GL_UNSIGNED_BYTE, face_->glyph->bitmap.buffer);
      // 设置纹理选项
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // 存储字符信息
      Character character = {
          texture,
          glm::ivec2(face_->glyph->bitmap.width, face_->glyph->bitmap.rows),
          glm::ivec2(face_->glyph->bitmap_left, face_->glyph->bitmap_top),
          static_cast<GLuint>(face_->glyph->advance.x)};
      characters_.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
  }
};
