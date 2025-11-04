#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "proxy.hpp"

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

static unsigned int CreateShader(const std::string& verticeshader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, verticeshader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
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
    window = glfwCreateWindow(1280, 720, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // enable experimental features for core contexts
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW init error\n";
        return -1;
    }
    std::cout << "GL_VENDOR: " << (const char*)glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL_RENDERER: " << (const char*)glGetString(GL_RENDERER) << std::endl;
    std::cout << "GL_VERSION: " << (const char*)glGetString(GL_VERSION) << std::endl;
    GLint maxTexSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    std::cout << "GL_MAX_TEXTURE_SIZE: " << maxTexSize << std::endl;

    {
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
        if (program == 0) {
            std::cerr << "Shader program creation failed\n";
            return -1;
        }
        std::cout << "Shader program created: " << program << std::endl;

        glUseProgram(program);
        
        float vertices[] = {
            // positions          // colors           // texture coords
            0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
            0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
            -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
        };
        unsigned int indices[] = {  
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };
        unsigned int VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        auto del_func = [VAO, VBO, EBO, program]()->void{
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
            glDeleteProgram(program);
        };
        Proxy<void, decltype(del_func)> opengl_cleanup(std::move(del_func));

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), 
                    vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // texture coord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load image, create texture and generate mipmaps
        int width, height, nrChannels;
        // flip vertically so texture coordinates match image origin if needed
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load("container.jpeg", &width, &height, &nrChannels, 0);
        std::cout
            << "stbi_load returned ptr=" << (void*)data
            << " w=" << width
            << " h=" << height
            << " ch=" << nrChannels
            << std::endl;
        if (data == nullptr)
        {
            std::cerr << "Failed to load texture\n";
            return -1;
        }
        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;
        std::cout << "Before glTexImage2D format=" << format << " size=" << width << "x" << height << std::endl;
        // ensure byte-aligned rows for image uploads
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLenum internalFormat = GL_RGB8;
        if (format == GL_RED) internalFormat = GL_R8;
        else if (format == GL_RGB) internalFormat = GL_RGB8;
        else if (format == GL_RGBA) internalFormat = GL_RGBA8;
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        std::cout << "After glTexImage2D" << std::endl;
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "After glGenerateMipmap" << std::endl;
        GLenum err = glGetError();
        std::cout << "glTexImage2D glGetError=" << err << std::endl;
        stbi_image_free(data);

        // set the sampler uniform to texture unit 0
        glUseProgram(program);
        std::cout << "After glUseProgram for sampler" << std::endl;
        GLint texLoc = glGetUniformLocation(program, "ourTexture");
        std::cout << "ourTexture location=" << texLoc << std::endl;
        if (texLoc >= 0) {
            glUniform1i(texLoc, 0);
            std::cout << "Sampler uniform set to 0" << std::endl;
        }

        unsigned int transformLoc = glGetUniformLocation(program, "transform");

        std::cout << "Entering render loop" << std::endl;
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            float time = (float)glfwGetTime();
            glm::mat4 trans = glm::mat4(1.0f);
            trans = glm::translate(trans, glm::vec3(0.0f, -0.5f * glm::sin(time), 0.0f));
            trans = glm::rotate(trans, time, glm::vec3(0.0f, 0.0f, 1.0f));
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

            // bind our texture to texture unit 0 before drawing
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
        std::cout << "Exited render loop" << std::endl;
    }

    glfwTerminate();
    return 0;
}
