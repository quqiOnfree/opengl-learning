#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;

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
        assert(OpenMesh::IO::read_mesh(mesh_, filename));

        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);
        glBindVertexArray(VAO_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);

        vertices_ = std::make_unique<float[]>(mesh_.n_faces() * 3 * 6);
        size_t idx = 0;

        MyMesh::Point minCoord = mesh_.point(MyMesh::VertexHandle(0));
        MyMesh::Point maxCoord = mesh_.point(MyMesh::VertexHandle(0));
        
        for (const auto& vh : mesh_.vertices()) {
            const auto& p = mesh_.point(vh);
            for (int i = 0; i < 3; ++i) {
                if (p[i] < minCoord[i]) minCoord[i] = p[i];
                if (p[i] > maxCoord[i]) maxCoord[i] = p[i];
            }
        }
        
        MyMesh::Point center = (minCoord + maxCoord) * 0.5f;
        float maxExtent = 0.0f;
        for (int i = 0; i < 3; ++i) {
            float extent = maxCoord[i] - minCoord[i];
            if (extent > maxExtent) maxExtent = extent;
        }
        
        float scale = 2.0f / maxExtent;

        for (const auto& face : mesh_.faces())
        {
            const auto& vh = face.vertices();
            auto iter = vh.begin();
            const auto& point1 = mesh_.point(iter++);
            vertices_[idx++] = (point1[0] - center[0]) * scale;
            vertices_[idx++] = (point1[1] - center[1]) * scale;
            vertices_[idx++] = -(point1[2] - center[2]) * scale;
            vertices_[idx++] = 1.0f; // R
            vertices_[idx++] = 0.0f; // G
            vertices_[idx++] = 0.0f; // B

            const auto& point2 = mesh_.point(iter++);
            vertices_[idx++] = (point2[0] - center[0]) * scale;
            vertices_[idx++] = (point2[1] - center[1]) * scale;
            vertices_[idx++] = -(point2[2] - center[2]) * scale;
            vertices_[idx++] = 0.0f; // R
            vertices_[idx++] = 1.0f; // G
            vertices_[idx++] = 0.0f; // B

            const auto& point3 = mesh_.point(iter++);
            vertices_[idx++] = (point3[0] - center[0]) * scale;
            vertices_[idx++] = (point3[1] - center[1]) * scale;
            vertices_[idx++] = -(point3[2] - center[2]) * scale;
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
    }

    void draw()
    {
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
    window = glfwCreateWindow(1280, 720, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW init error\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

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

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
