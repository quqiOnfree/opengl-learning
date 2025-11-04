#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;

// 全局网格对象
extern MyMesh mesh;

// 网格加载函数
int loadMesh(const char* filename);
void getMeshVertices(float** vertices, int* vertexCount);
void getMeshIndices(unsigned int** indices, int* indexCount);
void updateMeshBuffers(unsigned int VAO, unsigned int VBO, unsigned int EBO);

#endif