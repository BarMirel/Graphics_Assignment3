#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <Debugger.h>
#include <Shader.h>

class Camera
{
    private:
        // View and Projection
        glm::mat4 m_View = glm::mat4(1.0f);
        glm::mat4 m_Projection = glm::mat4(1.0f);

        // View matrix paramters
        glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 m_Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

        // Projection matrix parameters
        float m_Near = 0.1f; 
        float m_Far = 100.0f;
        int m_Width;
        int m_Height;

        // Orthographic Projection parameters
        float m_Left = -1.0f;
        float m_Right = 1.0f;
        float m_Bottom = -1.0f; 
        float m_Top = 1.0f;

        float m_FOV = 45.0f;
    public:
        // Prevent the camera from jumping around when first clicking left click
        double m_OldMouseX = 0.0;
        double m_OldMouseY = 0.0;
        double m_NewMouseX = 0.0;
        double m_NewMouseY = 0.0;
    public:
        Camera(int width, int height)
            : m_Width(width), m_Height(height) {};

        // Update Projection matrix for Orthographic mode
        void SetOrthographic(float near, float far);

        // Update Projection matrix for Perspective mode
        void SetPerspective(float fov, float near, float far);

        // Update window size 
        void SetWindowSize(int width, int height);

        void SetPosition(const glm::vec3& position);
        void SetOrientation(const glm::vec3& orientation);

        void RotateAroundOrigin(float deltaX, float deltaY);  // Rotate camera around origin 
        void PanCamera(float deltaX, float deltaY);            // Pan camera left/right/up/down
        void ZoomCamera(float deltaZ);                         // Move camera forward/backward

        // Handle camera inputs
        void EnableInputs(GLFWwindow* window);

        inline glm::mat4 GetViewMatrix() const { return m_View; }
        inline glm::mat4 GetProjectionMatrix() const { return m_Projection; }
        inline glm::vec3 GetOrientation() const { return m_Orientation; }
        inline glm::vec3 GetUp() const { return m_Up; }
        inline float GetNear() const { return m_Near; }
        inline float GetFar() const { return m_Far; }
        inline float GetFOV() const { return m_FOV; }
};