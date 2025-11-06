#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;

constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

class Camera
{
    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;

public:
    Camera():
        position_(0.0f, 0.0f, 3.0f),
        front_(0.0f, 0.0f, -1.0f),
        up_(0.0f, 1.0f, 0.0f)
    {}

    ~Camera() = default;

    glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(position_, position_ + front_, up_);
    }

    void processKeyboardInput(int key, float deltaTime)
    {
        float velocity = 2.5f * deltaTime;
        if (key == GLFW_KEY_W)
            position_ += front_ * velocity;
        if (key == GLFW_KEY_S)
            position_ -= front_ * velocity;
        if (key == GLFW_KEY_A)
            position_ -= glm::normalize(glm::cross(front_, up_)) * velocity;
        if (key == GLFW_KEY_D)
            position_ += glm::normalize(glm::cross(front_, up_)) * velocity;
        if (key == GLFW_KEY_SPACE)
            position_ += up_ * velocity;
        if (key == GLFW_KEY_LEFT_SHIFT)
            position_ -= up_ * velocity;
    }

    glm::vec3 getPosition() const { return position_; }
    glm::vec3 getFront() const { return front_; }
    glm::vec3 getUp() const { return up_; }

    void setPosition(const glm::vec3& position) { position_ = position; }
    void setFront(const glm::vec3& front) { front_ = front; }
    void setUp(const glm::vec3& up) { up_ = up; }
};

static unsigned int CompileShader(int type, const std::string& source)
{
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* message = static_cast<char*>(alloca(length + 1));
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

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
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

class mesh_loader
{
public:
    mesh_loader(const std::string& filename):
        vertices_(nullptr), VAO_(0), VBO_(0)
    {
        if (!OpenMesh::IO::read_mesh(mesh_, filename))
        {
            throw std::runtime_error("Error: Cannot read mesh from " + filename);
        }

        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);
        glBindVertexArray(VAO_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);

        vertices_ = std::make_unique<float[]>(mesh_.n_faces() * 3 * 6);
        size_t idx = 0;

        for (const auto& face : mesh_.faces())
        {
            const auto& vh = face.vertices();
            auto iter = vh.begin();
            const auto& point1 = mesh_.point(iter++);
            vertices_[idx++] = point1[0];
            vertices_[idx++] = point1[1];
            vertices_[idx++] = point1[2];
            vertices_[idx++] = 1.0f; // R
            vertices_[idx++] = 0.0f; // G
            vertices_[idx++] = 0.0f; // B

            const auto& point2 = mesh_.point(iter++);
            vertices_[idx++] = point2[0];
            vertices_[idx++] = point2[1];
            vertices_[idx++] = point2[2];
            vertices_[idx++] = 0.0f; // R
            vertices_[idx++] = 1.0f; // G
            vertices_[idx++] = 0.0f; // B

            const auto& point3 = mesh_.point(iter++);
            vertices_[idx++] = point3[0];
            vertices_[idx++] = point3[1];
            vertices_[idx++] = point3[2];
            vertices_[idx++] = 0.0f; // R
            vertices_[idx++] = 0.0f; // G
            vertices_[idx++] = 1.0f; // B
        }

        glBufferData(GL_ARRAY_BUFFER, mesh_.n_faces() * 3 * 6 * sizeof(float), vertices_.get(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~mesh_loader() noexcept {
        release();
    }

    void release() {
        if (VAO_) {
            glDeleteVertexArrays(1, &VAO_);
            VAO_ = 0;
        }
        if (VBO_) {
            glDeleteBuffers(1, &VBO_);
            VBO_ = 0;
        }
        vertices_.reset();
        mesh_.clear();
    }

    void draw()
    {
        if (!VAO_) return;
        glBindVertexArray(VAO_);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh_.n_faces() * 3));
        glBindVertexArray(0);
    }
    
    const MyMesh& get_mesh() const { return mesh_; }

private:
    MyMesh mesh_;
    std::unique_ptr<float[]> vertices_;
    unsigned int VAO_, VBO_;
};

Camera camera;

void processKeyboardInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboardInput(GLFW_KEY_W, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboardInput(GLFW_KEY_S, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboardInput(GLFW_KEY_A, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboardInput(GLFW_KEY_D, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processKeyboardInput(GLFW_KEY_SPACE, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.processKeyboardInput(GLFW_KEY_LEFT_SHIFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static float lastX = xpos, lastY = ypos;
    static float yaw = -90.0f;
    static float pitch = 0.0f;

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera.setFront(glm::normalize(front));
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        std::cerr << "GLFW init error\n";
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glEnable(GL_DEPTH_TEST);

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW init error\n";
        return -1;
    }

    std::ifstream vertex_file("vertex.glsl");
    std::ifstream fragment_file("fragment.glsl");
    if (!vertex_file || !fragment_file) {
        std::cerr << "vertex and fragment shader included error\n";
        return -1;
    }
    std::stringstream vertex_stream;
    std::stringstream fragment_stream;
    vertex_stream << vertex_file.rdbuf();
    fragment_stream << fragment_file.rdbuf();
    unsigned int program = CreateShader(vertex_stream.str(), fragment_stream.str());

    mesh_loader loader("skull.stl");

    glUseProgram(program);

    float lastFrame = (float)glfwGetTime();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = (float)glfwGetTime();
        float deltaTime = time - lastFrame;
        lastFrame = time;
        processKeyboardInput(window, deltaTime);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        GLint modelLoc = glGetUniformLocation(program, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glm::mat4 view = camera.getViewMatrix();
        GLint viewLoc = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), SCR_WIDTH * 1.0f / SCR_HEIGHT, 0.1f, 100.0f);
        GLint projectionLoc = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        loader.draw();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    loader.release();
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}
