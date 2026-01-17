#include <Camera.h>
#include <RubiksCube.h>
#include <Shader.h>
#include <VertexArray.h>
#include <VertexBuffer.h>
#include <IndexBuffer.h>
#include <Texture.h>
#include <glad/glad.h>
#include <iostream>
#include <cmath>

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
    
    // Function pointer for creating cubie vertices - for picking rendering
    void (*createCubieVertices)(const Cubie&, float*) = nullptr;
};

// Framebuffer size callback for window resizing
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    InputData* inputData = (InputData*) glfwGetWindowUserPointer(window);
    if (!inputData || !inputData->camera) {
        return;
    }
    
    // Update viewport
    GLCall(glViewport(0, 0, width, height));
    inputData->camera->SetWindowSize(width, height);
}

void Camera::SetOrthographic(float near, float far)
{
    m_Near = near;
    m_Far = far;

    // Rest Projection and View matrices
    m_Projection = glm::ortho(m_Left, m_Right, m_Bottom, m_Top, near, far);
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::SetPerspective(float fov, float near, float far)
{
    m_FOV = fov;
    m_Near = near;
    m_Far = far;

    float aspectRatio = (float)m_Width / (float)m_Height;
    m_Projection = glm::perspective(glm::radians(fov), aspectRatio, near, far);
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::SetWindowSize(int width, int height)
{
    m_Width = width;
    m_Height = height;
    
    // Only update if we're using perspective projection
    if (m_FOV > 0.0f) {
        float aspectRatio = (float)m_Width / (float)m_Height;
        m_Projection = glm::perspective(glm::radians(m_FOV), aspectRatio, m_Near, m_Far);
    }
}

void Camera::SetPosition(const glm::vec3& position)
{
    m_Position = position;
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::SetOrientation(const glm::vec3& orientation)
{
    m_Orientation = orientation;
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::RotateAroundOrigin(float deltaX, float deltaY)
{
    const float rotationSpeed = 0.5f;
    
    float yawAngle = -deltaX * rotationSpeed;
    float pitchAngle = deltaY * rotationSpeed;
    
    float distance = glm::length(m_Position);
    
    glm::vec3 direction = m_Position / distance;
    
    // Rotate direction around global Y axis (yaw)
    glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), glm::radians(yawAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    direction = glm::normalize(glm::vec3(yawRotation * glm::vec4(direction, 0.0f)));
    
    // Rotate direction around global X axis (pitch)
    glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));    
    glm::mat4 pitchRotation = glm::rotate(glm::mat4(1.0f), glm::radians(pitchAngle), right);
    direction = glm::normalize(glm::vec3(pitchRotation * glm::vec4(direction, 0.0f)));
    
    // Update camera position while maintaining distance from origin 
    m_Position = direction * distance;
    
    // Update orientation to look at origin
    m_Orientation = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - m_Position);
    
    right = glm::normalize(glm::cross(m_Orientation, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_Up = glm::normalize(glm::cross(right, m_Orientation));
    m_View = glm::lookAt(m_Position, glm::vec3(0.0f, 0.0f, 0.0f), m_Up);
}

void Camera::PanCamera(float deltaX, float deltaY)
{
    const float panSpeed = 0.01f;
    glm::vec3 right = glm::normalize(glm::cross(m_Orientation, m_Up));
    
    m_Position += right * deltaX * panSpeed;
    m_Position += m_Up * deltaY * panSpeed;
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::ZoomCamera(float deltaZ)
{
    const float zoomSpeed = 0.1f;
    m_Position += m_Orientation * deltaZ * zoomSpeed;
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

/////////////////////
// Input Callbacks //
/////////////////////

void KeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
    InputData* inputData = (InputData*) glfwGetWindowUserPointer(window);
    if (!inputData || !inputData->camera || !inputData->rubiksCube) {
        std::cout << "Warning: InputData wasn't set as the Window User Pointer! KeyCallback is skipped" << std::endl;
        return;
    }

    Camera* camera = inputData->camera;
    RubiksCube* rubiksCube = inputData->rubiksCube;

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
            case GLFW_KEY_R:
                std::cout << "R - Rotating Right Wall" << std::endl;
                rubiksCube->RotateRightWall();
                break;
            case GLFW_KEY_L:
                std::cout << "L - Rotating Left Wall" << std::endl;
                rubiksCube->RotateLeftWall();
                break;
            case GLFW_KEY_U:
                std::cout << "U - Rotating Up Wall" << std::endl;
                rubiksCube->RotateUpWall();
                break;
            case GLFW_KEY_D:
                std::cout << "D - Rotating Down Wall" << std::endl;
                rubiksCube->RotateDownWall();
                break;
            case GLFW_KEY_B:
                std::cout << "B - Rotating Back Wall" << std::endl;
                rubiksCube->RotateBackWall();
                break;
            case GLFW_KEY_F:
                std::cout << "F - Rotating Front Wall" << std::endl;
                rubiksCube->RotateFrontWall();
                break;
            case GLFW_KEY_SPACE:
                std::cout << "SPACE - Flipping Rotation Direction" << std::endl;
                rubiksCube->FlipRotationDirection();
                break;
            case GLFW_KEY_Z:
                std::cout << "Z - Dividing Rotation Angle" << std::endl;
                rubiksCube->DivideRotationAngle();
                std::cout << "New rotation angle: " << rubiksCube->GetRotationAngle() << " degrees" << std::endl;
                break;
            case GLFW_KEY_A:
                std::cout << "A - Multiplying Rotation Angle" << std::endl;
                rubiksCube->MultiplyRotationAngle();
                std::cout << "New rotation angle: " << rubiksCube->GetRotationAngle() << " degrees" << std::endl;
                break;
            case GLFW_KEY_UP:
                camera->RotateAroundOrigin(0.0f, 5.0f);
                break;
            case GLFW_KEY_DOWN:
                camera->RotateAroundOrigin(0.0f, -5.0f);
                break;
            case GLFW_KEY_LEFT:
                camera->RotateAroundOrigin(5.0f, 0.0f);
                break;
            case GLFW_KEY_RIGHT:
                camera->RotateAroundOrigin(-5.0f, 0.0f);
                break;
            case GLFW_KEY_P:
                inputData->pickingMode = !inputData->pickingMode;
                inputData->selectedCubieIndex = -1;
                std::cout << "Picking mode: " << (inputData->pickingMode ? "ON" : "OFF") << std::endl;
                break;
            default:
                break;
        }
    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    InputData* inputData = (InputData*) glfwGetWindowUserPointer(window);
    if (!inputData || !inputData->camera || !inputData->rubiksCube) {
        return;
    }
    Camera* camera = inputData->camera;
    RubiksCube* rubiksCube = inputData->rubiksCube;
    
    if (action == GLFW_PRESS) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        camera->m_OldMouseX = mouseX;
        camera->m_OldMouseY = mouseY;
        
        if (inputData->pickingMode && inputData->shader && inputData->va && inputData->vb && inputData->ib) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            
            // Clear the window
            GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
            GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            
            glm::mat4 view = camera->GetViewMatrix();
            glm::mat4 proj = camera->GetProjectionMatrix();
            
            inputData->shader->Bind();
            inputData->va->Bind();
            inputData->vb->Bind();
            inputData->ib->Bind();
            
            inputData->shader->SetUniform1i("u_PickingMode", 1);
            
            const std::vector<Cubie>& cubies = rubiksCube->GetCubies();
            float cubieVertices[24 * 8];
            
            for (size_t i = 0; i < cubies.size(); i++) {
                if (inputData->createCubieVertices) {
                    inputData->createCubieVertices(cubies[i], cubieVertices);
                }
                
                // Update vertex buffer
                GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cubieVertices), cubieVertices));
                
                glm::mat4 model = rubiksCube->GetModelMatrix((int)i);
                glm::mat4 mvp = proj * view * model;
                inputData->shader->SetUniformMat4f("u_MVP", mvp);
                
                const int shapeID = (int)i;
                const int packed = shapeID + 1;
                const int rByte = (packed & 0x000000FF) >> 0;
                const int gByte = (packed & 0x0000FF00) >> 8;
                const int bByte = (packed & 0x00FF0000) >> 16;
                glm::vec4 pickingColor(
                    rByte / 255.0f,
                    gByte / 255.0f,
                    bByte / 255.0f,
                    1.0f
                );
                inputData->shader->SetUniform4f("u_Color", pickingColor);
                
                // Draw the cubie
                GLCall(glDrawElements(GL_TRIANGLES, inputData->ib->GetCount(), GL_UNSIGNED_INT, nullptr));
            }

            int pixelX = (int)mouseX;
            int pixelY = height - (int)mouseY - 1;
            
            unsigned char pixel[4];
            GLCall(glReadPixels(pixelX, pixelY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel));
            
            const int colorId = (int)pixel[0] | ((int)pixel[1] << 8) | ((int)pixel[2] << 16);
            const int pickedIndex = colorId - 1;
            if (pickedIndex >= 0 && pickedIndex < (int)cubies.size()) {
                inputData->selectedCubieIndex = pickedIndex;
                std::cout << "Picked cubie index: " << pickedIndex << std::endl;
            } else {
                inputData->selectedCubieIndex = -1;
                std::cout << "No cubie picked (background clicked)" << std::endl;
            }
            
            float depth;
            GLCall(glReadPixels(pixelX, pixelY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth));
            
            if (depth > 0.0f && depth < 1.0f) {
                inputData->pickedDepth = depth;
                
                if (pickedIndex >= 0 && pickedIndex < (int)cubies.size()) {
                    inputData->initialPickMouseX = mouseX;
                    inputData->initialPickMouseY = mouseY;
                    inputData->initialPickWorldPos = cubies[pickedIndex].position;
                    inputData->isDragging = false;
                }
            } else {
                inputData->selectedCubieIndex = -1;
                inputData->pickedDepth = 0.0f;
                inputData->isDragging = false;
            }
            
            inputData->shader->SetUniform1i("u_PickingMode", 0);
        } else {
            inputData->isDragging = false;
        }
    } else if (action == GLFW_RELEASE) {
        inputData->isDragging = false;
    }
}

void CursorPosCallback(GLFWwindow* window, double currMouseX, double currMouseY)
{
    InputData* inputData = (InputData*) glfwGetWindowUserPointer(window);
    if (!inputData || !inputData->camera || !inputData->rubiksCube) {
        return;
    }
    Camera* camera = inputData->camera;
    RubiksCube* rubiksCube = inputData->rubiksCube;

    float deltaX = (float)(currMouseX - camera->m_OldMouseX);
    float deltaY = (float)(currMouseY - camera->m_OldMouseY);
    
    camera->m_OldMouseX = currMouseX;
    camera->m_OldMouseY = currMouseY;

    if (inputData->pickingMode && inputData->selectedCubieIndex >= 0) {
        Cubie& selectedCubie = rubiksCube->GetCubie(inputData->selectedCubieIndex);
        
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            
            double mouseDeltaX = mouseX - inputData->initialPickMouseX;
            double mouseDeltaY = mouseY - inputData->initialPickMouseY;
            double mouseMoveThreshold = 1.0;
            
            if (!inputData->isDragging) {
                if (std::abs(mouseDeltaX) < mouseMoveThreshold && std::abs(mouseDeltaY) < mouseMoveThreshold) {
                    return;  
                }
                inputData->isDragging = true;
            }
            glm::mat4 view = camera->GetViewMatrix();
            glm::mat4 proj = camera->GetProjectionMatrix();
            
            float depth = inputData->pickedDepth;
            
            if (depth <= 0.0f || depth >= 1.0f) {
                return;
            }
            
            float normalizedX = (2.0f * (float)mouseX) / (float)width - 1.0f;
            float normalizedY = 1.0f - (2.0f * (float)mouseY) / (float)height;
            
            glm::vec4 normalizedPoint(normalizedX, normalizedY, depth * 2.0f - 1.0f, 1.0f);
            
            // Unproject: transform from normalized coordinates to world space
            // World = inverse(Projection * View) * NormalizedPoint
            glm::mat4 invMVP = glm::inverse(proj * view);
            glm::vec4 worldPoint = invMVP * normalizedPoint;
            
            // Perspective divide 
            if (std::abs(worldPoint.w) > 0.0001f) {
                worldPoint /= worldPoint.w;
            }
            
            float initialNormalizedX = (2.0f * (float)inputData->initialPickMouseX) / (float)width - 1.0f;
            float initialNormalizedY = 1.0f - (2.0f * (float)inputData->initialPickMouseY) / (float)height;
            glm::vec4 initialNormalizedPoint(initialNormalizedX, initialNormalizedY, depth * 2.0f - 1.0f, 1.0f);
            glm::vec4 initialWorldPoint = invMVP * initialNormalizedPoint;
            if (std::abs(initialWorldPoint.w) > 0.0001f) {
                initialWorldPoint /= initialWorldPoint.w;
            }
            glm::vec3 initialWorldPosAtDepth(initialWorldPoint.x, initialWorldPoint.y, initialWorldPoint.z);
            
            glm::vec3 newWorldPos(worldPoint.x, worldPoint.y, worldPoint.z);
            glm::vec3 worldOffset = newWorldPos - initialWorldPosAtDepth;
            glm::vec3 newPosition = inputData->initialPickWorldPos + worldOffset;
            rubiksCube->TranslateCubie(inputData->selectedCubieIndex, newPosition - selectedCubie.position);
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            glm::vec3 cameraOrientation = camera->GetOrientation();
            glm::vec3 cameraUp = camera->GetUp();
            glm::vec3 cameraRight = glm::normalize(glm::cross(cameraOrientation, cameraUp));
            cameraUp = glm::normalize(glm::cross(cameraRight, cameraOrientation));
            
            const float rotationSpeed = 0.01f;
            float rotationX = deltaY * rotationSpeed;
            float rotationY = deltaX * rotationSpeed;
            
            glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), rotationX, cameraRight);
            glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), rotationY, cameraUp);
            glm::mat4 rotationDelta = rotY * rotX;
            
            rubiksCube->RotateCubie(inputData->selectedCubieIndex, rotationDelta);
        }
    } else {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            camera->RotateAroundOrigin(deltaX, deltaY);
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            camera->PanCamera(deltaX, -deltaY);
        }
    }
}

void ScrollCallback(GLFWwindow* window, double scrollOffsetX, double scrollOffsetY)
{
    InputData* inputData = (InputData*) glfwGetWindowUserPointer(window);
    if (!inputData || !inputData->camera) {
        return;
    }
    Camera* camera = inputData->camera;

    camera->ZoomCamera((float)scrollOffsetY);
}

void Camera::EnableInputs(GLFWwindow* window)
{
    // Handle key inputs
    glfwSetKeyCallback(window, (void(*)(GLFWwindow *, int, int, int, int)) KeyCallback);

    // Handle cursor buttons
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    // Handle cursor position and inputs on motion
    glfwSetCursorPosCallback(window , (void(*)(GLFWwindow *, double, double)) CursorPosCallback);

    // Handle scroll inputs
    glfwSetScrollCallback(window, (void(*)(GLFWwindow *, double, double)) ScrollCallback);
    
    // Handle window resize
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
}