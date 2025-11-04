#include "mesh_data.h"
#include <stdlib.h>
#include <GL/glew.h>

typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;

MyMesh mesh;

int loadMesh(const char* filename) {
    // 清除现有网格数据
    mesh.clear();

    // 加载网格文件
    OpenMesh::IO::Options opt;
    if (!OpenMesh::IO::read_mesh(mesh, filename, opt)) {
        return 0; // 加载失败
    }

    // 请求顶点法线（如果需要）
    if (!opt.check(OpenMesh::IO::Options::VertexNormal)) {
        mesh.request_vertex_normals();
    }

    // 更新网格属性
    mesh.update_normals();

    return 1; // 加载成功
}

void getMeshVertices(float** vertices, int* vertexCount) {
    *vertexCount = mesh.n_vertices() * 6; // 位置(3) + 颜色(3)
    *vertices = (float*)malloc(*vertexCount * sizeof(float));

    int index = 0;
    for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it) {
        // 位置数据
        MyMesh::Point point = mesh.point(*v_it);
        (*vertices)[index++] = point[0];
        (*vertices)[index++] = point[1];
        (*vertices)[index++] = point[2];

        // 颜色数据（可以根据需要修改）
        (*vertices)[index++] = 1.0f; // R
        (*vertices)[index++] = 0.5f; // G
        (*vertices)[index++] = 0.2f; // B
    }
}

void getMeshIndices(unsigned int** indices, int* indexCount) {
    *indexCount = mesh.n_faces() * 3; // 三角形面片
    *indices = (unsigned int*)malloc(*indexCount * sizeof(unsigned int));

    int index = 0;
    for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it) {
        for (MyMesh::FaceVertexIter fv_it = mesh.fv_iter(*f_it); fv_it.is_valid(); ++fv_it) {
            (*indices)[index++] = fv_it->idx();
        }
    }
}

void updateMeshBuffers(unsigned int VAO, unsigned int VBO, unsigned int EBO) {
    float* vertices;
    unsigned int* indices;
    int vertexCount, indexCount;

    // 获取最新的网格数据
    getMeshVertices(&vertices, &vertexCount);
    getMeshIndices(&indices, &indexCount);

    // 绑定VAO
    glBindVertexArray(VAO);

    // 更新顶点缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);

    // 更新索引缓冲区
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // 设置顶点属性指针（只需要设置一次，但重新设置也无妨）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 释放内存
    free(vertices);
    free(indices);

    printf("Mesh buffers updated: %d vertices, %d indices\n", vertexCount / 6, indexCount);
}