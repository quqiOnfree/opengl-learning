#ifndef CAMERA_H
#define CAMERA_H

#include <math.h>

#define PI 3.14159265359f

// 相机移动选项
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// 默认相机值
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

typedef struct Camera {
    // 相机属性
    float Position[3];
    float Front[3];
    float Up[3];
    float Right[3];
    float WorldUp[3];

    // 欧拉角
    float Yaw;
    float Pitch;

    // 相机选项
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
} Camera;

void updateCameraVectors(Camera* camera) {
    // 计算新的Front向量
    float front[3];
    front[0] = cosf(camera->Yaw * PI / 180.0f) * cosf(camera->Pitch * PI / 180.0f);
    front[1] = sinf(camera->Pitch * PI / 180.0f);
    front[2] = sinf(camera->Yaw * PI / 180.0f) * cosf(camera->Pitch * PI / 180.0f);

    // 归一化向量
    float length = sqrtf(front[0] * front[0] + front[1] * front[1] + front[2] * front[2]);
    camera->Front[0] = front[0] / length;
    camera->Front[1] = front[1] / length;
    camera->Front[2] = front[2] / length;

    // 重新计算Right和Up向量
    // Right = Front × WorldUp
    camera->Right[0] = camera->Front[1] * camera->WorldUp[2] - camera->Front[2] * camera->WorldUp[1];
    camera->Right[1] = camera->Front[2] * camera->WorldUp[0] - camera->Front[0] * camera->WorldUp[2];
    camera->Right[2] = camera->Front[0] * camera->WorldUp[1] - camera->Front[1] * camera->WorldUp[0];

    // 归一化Right向量
    length = sqrtf(camera->Right[0] * camera->Right[0] + camera->Right[1] * camera->Right[1] + camera->Right[2] * camera->Right[2]);
    camera->Right[0] /= length;
    camera->Right[1] /= length;
    camera->Right[2] /= length;

    // Up = Right × Front
    camera->Up[0] = camera->Right[1] * camera->Front[2] - camera->Right[2] * camera->Front[1];
    camera->Up[1] = camera->Right[2] * camera->Front[0] - camera->Right[0] * camera->Front[2];
    camera->Up[2] = camera->Right[0] * camera->Front[1] - camera->Right[1] * camera->Front[0];
}

void initCamera(Camera* camera) {
    camera->Position[0] = 0.0f;
    camera->Position[1] = 0.0f;
    camera->Position[2] = 3.0f;

    camera->WorldUp[0] = 0.0f;
    camera->WorldUp[1] = 1.0f;
    camera->WorldUp[2] = 0.0f;

    camera->Yaw = YAW;
    camera->Pitch = PITCH;

    camera->MovementSpeed = SPEED;
    camera->MouseSensitivity = SENSITIVITY;
    camera->Zoom = ZOOM;

    updateCameraVectors(camera);
}

void getViewMatrix(Camera* camera, float* viewMatrix) {
    float center[3];
    center[0] = camera->Position[0] + camera->Front[0];
    center[1] = camera->Position[1] + camera->Front[1];
    center[2] = camera->Position[2] + camera->Front[2];

    float f[3] = { camera->Front[0], camera->Front[1], camera->Front[2] };
    float s[3] = { camera->Right[0], camera->Right[1], camera->Right[2] };
    float u[3] = { camera->Up[0], camera->Up[1], camera->Up[2] };

    viewMatrix[0] = s[0]; viewMatrix[1] = u[0]; viewMatrix[2] = -f[0]; viewMatrix[3] = 0.0f;
    viewMatrix[4] = s[1]; viewMatrix[5] = u[1]; viewMatrix[6] = -f[1]; viewMatrix[7] = 0.0f;
    viewMatrix[8] = s[2]; viewMatrix[9] = u[2]; viewMatrix[10] = -f[2]; viewMatrix[11] = 0.0f;
    viewMatrix[12] = -s[0] * camera->Position[0] - s[1] * camera->Position[1] - s[2] * camera->Position[2];
    viewMatrix[13] = -u[0] * camera->Position[0] - u[1] * camera->Position[1] - u[2] * camera->Position[2];
    viewMatrix[14] = f[0] * camera->Position[0] + f[1] * camera->Position[1] + f[2] * camera->Position[2];
    viewMatrix[15] = 1.0f;
}

void processKeyboard(Camera* camera, enum Camera_Movement direction, float deltaTime) {
    float velocity = camera->MovementSpeed * deltaTime;

    switch (direction) {
    case FORWARD:
        camera->Position[0] += camera->Front[0] * velocity;
        camera->Position[1] += camera->Front[1] * velocity;
        camera->Position[2] += camera->Front[2] * velocity;
        break;
    case BACKWARD:
        camera->Position[0] -= camera->Front[0] * velocity;
        camera->Position[1] -= camera->Front[1] * velocity;
        camera->Position[2] -= camera->Front[2] * velocity;
        break;
    case LEFT:
        camera->Position[0] -= camera->Right[0] * velocity;
        camera->Position[1] -= camera->Right[1] * velocity;
        camera->Position[2] -= camera->Right[2] * velocity;
        break;
    case RIGHT:
        camera->Position[0] += camera->Right[0] * velocity;
        camera->Position[1] += camera->Right[1] * velocity;
        camera->Position[2] += camera->Right[2] * velocity;
        break;
    case UP:
        camera->Position[0] += camera->Up[0] * velocity;
        camera->Position[1] += camera->Up[1] * velocity;
        camera->Position[2] += camera->Up[2] * velocity;
        break;
    case DOWN:
        camera->Position[0] -= camera->Up[0] * velocity;
        camera->Position[1] -= camera->Up[1] * velocity;
        camera->Position[2] -= camera->Up[2] * velocity;
        break;
    }
}

void processMouseMovement(Camera* camera, float xoffset, float yoffset) {
    // 移除默认参数，直接使用约束
    int constrainPitch = 1;

    xoffset *= camera->MouseSensitivity;
    yoffset *= camera->MouseSensitivity;

    camera->Yaw += xoffset;
    camera->Pitch += yoffset;

    // 确保当俯仰角超出边界时屏幕不会被翻转
    if (constrainPitch) {
        if (camera->Pitch > 89.0f)
            camera->Pitch = 89.0f;
        if (camera->Pitch < -89.0f)
            camera->Pitch = -89.0f;
    }

    updateCameraVectors(camera);
}

void processMouseScroll(Camera* camera, float yoffset) {
    camera->Zoom -= yoffset;
    if (camera->Zoom < 1.0f)
        camera->Zoom = 1.0f;
    if (camera->Zoom > 45.0f)
        camera->Zoom = 45.0f;
}

void createModelMatrix(float* model, float angle) {
    // 单位矩阵
    for (int i = 0; i < 16; i++) model[i] = 0.0f;
    model[0] = 1.0f; model[5] = 1.0f; model[10] = 1.0f; model[15] = 1.0f;

    // 旋转
    float cosA = cosf(angle);
    float sinA = sinf(angle);

    model[0] = cosA; model[2] = sinA;
    model[8] = -sinA; model[10] = cosA;
}

void createProjectionMatrix(float* projection, float fov, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov * PI / 360.0f);

    for (int i = 0; i < 16; i++) projection[i] = 0.0f;

    projection[0] = f / aspect;
    projection[5] = f;
    projection[10] = (far + near) / (near - far);
    projection[11] = -1.0f;
    projection[14] = (2.0f * far * near) / (near - far);
}

#endif
