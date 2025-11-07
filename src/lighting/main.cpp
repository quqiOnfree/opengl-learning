#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <camera.hpp>
#include <mesh_loader.hpp>
#include <shader.hpp>

constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

Camera camera;

void processKeyboardInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.moveForward(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.moveBackward(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.moveLeft(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.moveRight(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.moveUp(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.moveDown(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static float lastX = xpos, lastY = ypos;
    static float yaw = -90.0f;
    static float pitch = 0.0f;

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
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

    std::string vertex_source, fragment_source, light_fragment_source;
    // Load GLSL resouce
    {
        std::ifstream vertex_file("vertex.glsl");
        std::ifstream fragment_file("fragment.glsl");
        std::ifstream light_fragment_file("light_fragment.glsl");
        if (!vertex_file || !fragment_file || !light_fragment_file) {
            std::cerr << "vertex and fragment shader included error\n";
            return 0;
        }
        std::stringstream vertex_stream;
        std::stringstream fragment_stream;
        std::stringstream light_fragment_stream;
        vertex_stream << vertex_file.rdbuf();
        fragment_stream << fragment_file.rdbuf();
        light_fragment_stream << light_fragment_file.rdbuf();
        vertex_source = vertex_stream.str();
        fragment_source = fragment_stream.str();
        light_fragment_source = light_fragment_stream.str();
    }

    MyMesh mesh;
    {
        std::string filename = "cube.stl";
        if (!OpenMesh::IO::read_mesh(mesh, filename))
        {
            std::cerr << "Error: Cannot read mesh from " << filename << '\n';
            return 0;
        }
    }
    
    {
        Shader obj_shader(vertex_source, fragment_source);
        Shader light_shader(vertex_source, light_fragment_source);

        VertexBufferObject cube_vbo{mesh};
        VertexArrayObject obj_vao{cube_vbo, [](){
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
            glEnableVertexAttribArray(0);
        }};
        VertexArrayObject light_vao{cube_vbo, [](){
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
            glEnableVertexAttribArray(0);
        }};

        // init obj
        obj_vao.bind();
        cube_vbo.bind();
        cube_vbo.unbind();
        obj_vao.unbind();

        // init light
        light_vao.bind();
        cube_vbo.bind();
        cube_vbo.unbind();
        light_vao.unbind();

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), SCR_WIDTH * 1.0f / SCR_HEIGHT, 0.1f, 100.0f);

        glm::vec3 lightColor = {1.0f, 1.0f, 1.0f};

        // Set object of shader
        obj_shader.use();
        obj_shader.setUniform("projection", [projection](GLint loc){
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection));
        });
        obj_shader.setUniform("lightColor", [lightColor](GLint loc){
            glUniform3f(loc, 1.0f, 1.0f, 1.0f);
        });

        // Set light of shader
        light_shader.use();
        light_shader.setUniform("projection", [projection](GLint loc){
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection));
        });
        light_shader.setUniform("lightColor", [lightColor](GLint loc){
            glUniform3f(loc, 1.0f, 1.0f, 1.0f);
        });

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

            glm::mat4 view = camera.getViewMatrix();

            // Set object of shader
            obj_shader.use();
            obj_shader.setUniform("view", [view](GLint loc){
                glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
            });

            // Set light of shader
            light_shader.use();
            light_shader.setUniform("view", [view](GLint loc){
                glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
            });

            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
                model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

                light_shader.setUniform("model", [model](GLint loc){
                    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));
                });

                light_vao.bind();
                light_vao.draw();
                light_vao.unbind();
            }

            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::translate(model, glm::vec3(10.0f, 0.0f, 0.0f));
                // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
                model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
                
                obj_shader.use();
                obj_shader.setUniform("model", [model](GLint loc){
                    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));
                });
                obj_shader.setUniform("objectColor", [model](GLint loc){
                    glUniform3f(loc, 1.0f, 1.0f, 1.0f);
                });

                obj_vao.bind();
                obj_vao.draw();
                obj_vao.unbind();
            }

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}

