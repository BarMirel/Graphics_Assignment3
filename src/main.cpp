#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Debugger.h>
#include <VertexBuffer.h>
#include <VertexBufferLayout.h>
#include <IndexBuffer.h>
#include <VertexArray.h>
#include <Shader.h>
#include <Texture.h>
#include <Camera.h>
#include <RubiksCube.h>

#include <iostream>

// Forward declarations
class Shader;
class VertexArray;
class VertexBuffer;
class IndexBuffer;
class Texture;

struct InputData {
    Camera* camera;
    RubiksCube* rubiksCube;
    
    bool pickingMode = false;
    int selectedCubieIndex = -1;
    float pickedDepth = 0.0f;
    double initialPickMouseX = 0.0;
    double initialPickMouseY = 0.0;
    glm::vec3 initialPickWorldPos = glm::vec3(0.0f);
    bool isDragging = false;
    GLFWwindow* window = nullptr;
    Shader* shader = nullptr;
    VertexArray* va = nullptr;
    VertexBuffer* vb = nullptr;
    IndexBuffer* ib = nullptr;
    Texture* texture = nullptr;
    
    void (*createCubieVertices)(const Cubie&, float*) = nullptr;
};

/* Window size */
const unsigned int width = 800;
const unsigned int height = 800;
const float FOVdegree = 45.0f;
const float near = 0.1f;
const float far = 100.0f;

void CreateCubieVertices(const Cubie& cubie, float* vertices) {
    float baseVertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,  // 0
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,  // 1
         0.5f,  0.5f,  0.5f,    1.0f, 1.0f,  // 2
        -0.5f,  0.5f,  0.5f,    0.0f, 1.0f,  // 3
        // Back face
         0.5f, -0.5f, -0.5f,    0.0f, 0.0f,  // 4
        -0.5f, -0.5f, -0.5f,    1.0f, 0.0f,  // 5
        -0.5f,  0.5f, -0.5f,    1.0f, 1.0f,  // 6
         0.5f,  0.5f, -0.5f,    0.0f, 1.0f,  // 7
        // Top face
        -0.5f,  0.5f,  0.5f,    0.0f, 0.0f,  // 8
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,  // 9
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,  // 10
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,  // 11
        // Bottom face
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,  // 12
         0.5f, -0.5f, -0.5f,    1.0f, 0.0f,  // 13
         0.5f, -0.5f,  0.5f,    1.0f, 1.0f,  // 14
        -0.5f, -0.5f,  0.5f,    0.0f, 1.0f,  // 15
        // Right face
         0.5f, -0.5f,  0.5f,    0.0f, 0.0f,  // 16
         0.5f, -0.5f, -0.5f,    1.0f, 0.0f,  // 17
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,  // 18
         0.5f,  0.5f,  0.5f,    0.0f, 1.0f,  // 19
        // Left face
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,  // 20
        -0.5f, -0.5f,  0.5f,    1.0f, 0.0f,  // 21
        -0.5f,  0.5f,  0.5f,    1.0f, 1.0f,  // 22
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f   // 23
    };
    
    for (int i = 0; i < 24; i++) {
        int baseIdx = i * 5;
        int outIdx = i * 8;
        
        vertices[outIdx] = baseVertices[baseIdx];
        vertices[outIdx + 1] = baseVertices[baseIdx + 1];
        vertices[outIdx + 2] = baseVertices[baseIdx + 2];
        
        glm::vec3 color;
        if (i < 4) color = cubie.frontColor;
        else if (i < 8) color = cubie.backColor;
        else if (i < 12) color = cubie.topColor;
        else if (i < 16) color = cubie.bottomColor;
        else if (i < 20) color = cubie.rightColor;
        else color = cubie.leftColor;
        
        vertices[outIdx + 3] = color.x;
        vertices[outIdx + 4] = color.y;
        vertices[outIdx + 5] = color.z;
        
        vertices[outIdx + 6] = baseVertices[baseIdx + 3];
        vertices[outIdx + 7] = baseVertices[baseIdx + 4];
    }
}

/* Indices for cube (12 triangles = 36 indices) */
unsigned int cubeIndices[] = {
    // Front face
    0, 1, 2,  2, 3, 0,
    // Back face
    4, 5, 6,  6, 7, 4,
    // Top face
    8, 9, 10, 10, 11, 8,
    // Bottom face
    12, 13, 14, 14, 15, 12,
    // Right face
    16, 17, 18, 18, 19, 16,
    // Left face
    20, 21, 22, 22, 23, 20
};

int main(int argc, char* argv[])
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        return -1;
    }
    
    /* Set OpenGL to Version 3.3.0 */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Load GLAD so it configures OpenGL */
    gladLoadGL();
    
    /* Set initial viewport */
    GLCall(glViewport(0, 0, width, height));

    /* Control frame rate */
    glfwSwapInterval(1);

    /* Print OpenGL version after completing initialization */
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    /* Set scope so that on widow close the destructors will be called automatically */
    {
        /* Blend to fix images with transperancy */
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        /* Generate VAO, VBO, EBO and bind them */
        VertexArray va;
        float cubieVertices[24 * 8];
        VertexBuffer vb(cubieVertices, sizeof(cubieVertices));
        IndexBuffer ib(cubeIndices, sizeof(cubeIndices));

        VertexBufferLayout layout;
        layout.Push<float>(3);  // positions
        layout.Push<float>(3);  // colors
        layout.Push<float>(2);  // texCoords
        va.AddBuffer(vb, layout);

        /* Create texture */
        Texture texture("res/textures/plane.png");
        texture.Bind();
         
        /* Create shaders */
        Shader shader("res/shaders/basic.shader");
        shader.Bind();

        /* Unbind all to prevent accidentally modifying them */
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        /* Enables the Depth Buffer */
    	GLCall(glEnable(GL_DEPTH_TEST));

        /* Create camera */
        Camera camera(width, height);
        // Position camera to view the Rubik's cube from an angle
        camera.SetPosition(glm::vec3(3.0f, 3.0f, 3.0f));
        camera.SetOrientation(glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f)));
        camera.SetPerspective(FOVdegree, near, far);

        RubiksCube rubiksCube(0.3f, 0.01f);
        
        InputData inputData;
        inputData.camera = &camera;
        inputData.rubiksCube = &rubiksCube;
        inputData.window = window;
        inputData.shader = &shader;
        inputData.va = &va;
        inputData.vb = &vb;
        inputData.ib = &ib;
        inputData.texture = &texture;
        inputData.createCubieVertices = CreateCubieVertices;
        glfwSetWindowUserPointer(window, &inputData);
        
        camera.EnableInputs(window);

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Set white background color */
            GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

            /* Render here */
            GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            /* Initialize uniform color */
            glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

            /* Get view and projection matrices */
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 proj = camera.GetProjectionMatrix();

            /* Bind shader and texture once */
            shader.Bind();
            shader.SetUniform1i("u_PickingMode", 0);  
            shader.SetUniform4f("u_Color", color);
            shader.SetUniform1i("u_Texture", 0);
            texture.Bind();
            va.Bind();
            ib.Bind();

            const std::vector<Cubie>& cubies = rubiksCube.GetCubies();
            for (size_t i = 0; i < cubies.size(); i++) {
                CreateCubieVertices(cubies[i], cubieVertices);
                
                /* Update vertex buffer with this cubie's colors */
                vb.Bind();
                GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cubieVertices), cubieVertices));
                
                glm::mat4 model = rubiksCube.GetModelMatrix(i);
                glm::mat4 mvp = proj * view * model;
                shader.SetUniformMat4f("u_MVP", mvp);
                GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
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