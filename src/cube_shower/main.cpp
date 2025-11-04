#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "shader.h"
#include "camera.h"
#include "cube_data.h"
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include "mesh_data.h"

// 窗口设置
#define SCR_WIDTH 800
#define SCR_HEIGHT 600

// 全局变量
GLFWwindow* window;
Camera camera;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
int firstMouse = 1;

unsigned int meshVAO = 0, meshVBO = 0, meshEBO = 0;
int useMesh = 0; // 0=立方体, 1=网格
int meshInitialized = 0;
typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;

// 函数声明
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
int initGLFW();
int initGLEW();


int initGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    return 1;
}

int initGLEW() {
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "GLEW init error\n");
        return 0;
    }
    return 1;
}

void initMeshBuffers() {
    glGenVertexArrays(1, &meshVAO);
    glGenBuffers(1, &meshVBO);
    glGenBuffers(1, &meshEBO);

    updateMeshBuffers(meshVAO, meshVBO, meshEBO);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = 0;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // 反转Y坐标

    lastX = (float)xpos;
    lastY = (float)ypos;

    processMouseMovement(&camera, xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    processMouseScroll(&camera, (float)yoffset);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        processKeyboard(&camera, FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        processKeyboard(&camera, BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        processKeyboard(&camera, LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        processKeyboard(&camera, RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        processKeyboard(&camera, UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        processKeyboard(&camera, DOWN, deltaTime);

    // 添加网格切换和操作
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        useMesh = !useMesh; // 切换模型
        printf("Switched to %s\n", useMesh ? "mesh" : "cube");
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        // 重新加载网格
        if (loadMesh("model.obj")) {
            if (meshInitialized) {
                // 更新网格缓冲区
                updateMeshBuffers(meshVAO, meshVBO, meshEBO);
                printf("Mesh reloaded successfully\n");
            }
            else {
                // 首次初始化网格缓冲区
                initMeshBuffers();
                meshInitialized = 1;
                useMesh = 1;
                printf("Mesh loaded and initialized successfully\n");
            }
        }
        else {
            printf("Failed to reload mesh\n");
        }
    }
}


int main() {
    // 初始化GLFW
    if (!initGLFW()) {
        return -1;
    }

    // 创建窗口
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Cube - C Language", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!initGLEW()) {
        glfwTerminate();
        return -1;
    }

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 初始化相机
    initCamera(&camera);

    // 创建着色器程序
    unsigned int shaderProgram = createShaderProgram(
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "out vec3 ourColor;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "   ourColor = aColor;\n"
        "}\0",
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(ourColor, 1.0);\n"
        "}\0"
    );

    if (shaderProgram == 0) {
        printf("Failed to create shader program\n");
        return -1;
    }

    // 创建立方体VAO, VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 加载网格（只初始化一次！）
    if (loadMesh("model.obj")) {
        initMeshBuffers();  // 在这里初始化网格缓冲区
        meshInitialized = 1;
        useMesh = 1;
        printf("Mesh loaded successfully\n");
    }
    else {
        printf("Using default cube\n");
    }

    // 渲染循环（修正后的正确顺序）
    while (!glfwWindowShouldClose(window)) {
        // 计算每帧时间
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 处理输入
        processInput(window);

        // 渲染：先清除缓冲区
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 设置着色器和矩阵
        glUseProgram(shaderProgram);

        // 获取矩阵uniform的位置
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        // 模型矩阵：旋转物体
        float model[16];
        createModelMatrix(model, (float)glfwGetTime());
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);

        // 观察矩阵
        float view[16];
        getViewMatrix(&camera, view);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);

        // 投影矩阵
        float projection[16];
        createProjectionMatrix(projection, camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);

        // 根据条件绘制（只绘制一次！）
        if (useMesh && meshInitialized) {
            // 绘制网格
            glBindVertexArray(meshVAO);

            // 获取索引数量用于绘制
            unsigned int* indices;
            int indexCount;
            getMeshIndices(&indices, &indexCount);
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
            free(indices);
        }
        else {
            // 绘制立方体
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 交换缓冲区和检查事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    if (meshInitialized) {
        glDeleteVertexArrays(1, &meshVAO);
        glDeleteBuffers(1, &meshVBO);
        glDeleteBuffers(1, &meshEBO);
    }

    glfwTerminate();
    return 0;
}
