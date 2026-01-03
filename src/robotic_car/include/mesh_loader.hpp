#pragma once

#include <GL/glew.h>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

using MyMesh = OpenMesh::TriMesh_ArrayKernelT<>;

class MeshVertexBufferObject {
public:
  MeshVertexBufferObject(const MyMesh &mesh)
      : vertices_(nullptr), VBO_(0), n_faces_(mesh.n_faces()) {

    glGenBuffers(1, &VBO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);

    vertices_ =
        std::make_unique<float[]>( // NOLINT(cppcoreguidelines-avoid-c-arrays)
            mesh.n_faces() * 3 * 6);
    size_t idx = 0;

    for (const auto &face : mesh.faces()) {
      const auto &vh = face.vertices();
      auto iter = vh.begin();
      const auto &point1 = mesh.point(iter++);
      vertices_[idx++] = point1[0];
      vertices_[idx++] = point1[1];
      vertices_[idx++] = point1[2];

      const auto &point2 = mesh.point(iter++);
      vertices_[idx++] = point2[0];
      vertices_[idx++] = point2[1];
      vertices_[idx++] = point2[2];

      const auto &point3 = mesh.point(iter++);
      vertices_[idx++] = point3[0];
      vertices_[idx++] = point3[1];
      vertices_[idx++] = point3[2];
    }

    glBufferData(
        GL_ARRAY_BUFFER,
        mesh.n_faces() // NOLINT(cppcoreguidelines-narrowing-conversions)
            * 3 * 3 * sizeof(float),
        vertices_.get(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  template <std::size_t N>
  MeshVertexBufferObject(
      const float( // NOLINT(cppcoreguidelines-avoid-c-arrays)
          &matrix)[N][3][3]) {
    vertices_ =
        std::make_unique<float[]>( // NOLINT(cppcoreguidelines-avoid-c-arrays)
            N);
    size_t idx = 0;

    n_faces_ = N * 3;

    for (std::size_t i = 0; i < N; ++i) {
      vertices_[idx++] = matrix[i][0][0];
      vertices_[idx++] = matrix[i][0][1];
      vertices_[idx++] = matrix[i][0][2];

      vertices_[idx++] = matrix[i][1][0];
      vertices_[idx++] = matrix[i][1][1];
      vertices_[idx++] = matrix[i][1][2];

      vertices_[idx++] = matrix[i][2][0];
      vertices_[idx++] = matrix[i][2][1];
      vertices_[idx++] = matrix[i][2][2];
    }

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<
            long long>( // NOLINT(cppcoreguidelines-narrowing-conversions)
            n_faces_) *
            3 * 3 * sizeof(float),
        vertices_.get(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  ~MeshVertexBufferObject() noexcept { release(); }
  MeshVertexBufferObject(const MeshVertexBufferObject &) = delete;
  MeshVertexBufferObject &operator=(const MeshVertexBufferObject &) = delete;
  MeshVertexBufferObject(MeshVertexBufferObject &&VBO) noexcept
      : vertices_(std::move(VBO.vertices_)), n_faces_(VBO.n_faces_),
        VBO_(std::exchange(VBO.VBO_, 0u)) {}
  MeshVertexBufferObject &operator=(MeshVertexBufferObject &&VBO) noexcept {
    if (this != &VBO) {
      release();
      vertices_ = std::move(VBO.vertices_);
      n_faces_ = VBO.n_faces_;
      VBO_ = std::exchange(VBO.VBO_, 0u);
    }
    return *this;
  }

  void release() {
    if (VBO_) {
      glDeleteBuffers(1, &VBO_);
      VBO_ = 0;
    }
    vertices_.reset();
  }

  void reset(MeshVertexBufferObject &&VBO) {
    release();
    vertices_ = std::move(VBO.vertices_);
    VBO_ = std::exchange(VBO.VBO_, 0u);
  }

  void bind() const { glBindBuffer(GL_ARRAY_BUFFER, VBO_); }

  void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

  std::size_t n_faces() const { return n_faces_; }

private:
  std::unique_ptr<float[]> // NOLINT(cppcoreguidelines-avoid-c-arrays)
      vertices_;
  std::size_t n_faces_;
  unsigned int VBO_;
};

class VertexArrayObject {
public:
  template <typename Set>
  VertexArrayObject(const MeshVertexBufferObject &VBO, Set set) : VBO_(&VBO) {
    glGenVertexArrays(1, &VAO_);
    glBindVertexArray(VAO_);
    VBO_->bind();
    std::invoke(set);
    glBindVertexArray(0);
  }

  ~VertexArrayObject() { release(); }
  VertexArrayObject(const VertexArrayObject &) = delete;
  VertexArrayObject &operator=(const VertexArrayObject &) = delete;
  VertexArrayObject(VertexArrayObject &&VAO) noexcept
      : VAO_(VAO.VAO_), VBO_(VAO.VBO_) {
    VAO.VAO_ = 0;
  }
  VertexArrayObject &operator=(VertexArrayObject &&VAO) noexcept {
    if (this != &VAO) {
      release();
      VAO_ = VAO.VAO_;
      VBO_ = VAO.VBO_;
      VAO.VAO_ = 0;
    }
    return *this;
  }

  void release() {
    if (!VAO_) {
      return;
    }
    glDeleteVertexArrays(1, &VAO_);
  }

  void bind() const { glBindVertexArray(VAO_); }

  void unbind() const { glBindVertexArray(0); }

  void draw() const {
    bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(VBO_->n_faces() * 3));
  }

private:
  unsigned int VAO_;
  const MeshVertexBufferObject *VBO_;
};
