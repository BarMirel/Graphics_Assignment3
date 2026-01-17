#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

struct Cubie {
    int shapeIndex;  
    glm::vec3 position;  
    glm::mat4 rotation;
    glm::mat4 translation; 
    glm::vec3 initialGridPos;
    glm::vec3 frontColor;
    glm::vec3 backColor;
    glm::vec3 topColor;
    glm::vec3 bottomColor;
    glm::vec3 rightColor;
    glm::vec3 leftColor;
    
    Cubie(int index, glm::vec3 gridPos) 
        : shapeIndex(index), position(gridPos), 
          rotation(glm::mat4(1.0f)), translation(glm::translate(glm::mat4(1.0f), gridPos)),
          initialGridPos(gridPos) {

        frontColor = glm::vec3(1.0f, 0.0f, 0.0f);
        backColor = glm::vec3(1.0f, 0.5f, 0.0f);
        topColor = glm::vec3(1.0f, 1.0f, 0.0f);
        bottomColor = glm::vec3(1.0f, 1.0f, 1.0f);
        rightColor = glm::vec3(0.0f, 1.0f, 0.0f);
        leftColor = glm::vec3(0.0f, 0.0f, 1.0f);
    }
};

class RubiksCube {
private:
    std::vector<Cubie> m_Cubies;
    float m_CubeSize;
    float m_Spacing;
    float m_RotationAngle;
    bool m_Clockwise;
    void RotateLayer(int layer, char axis, float angle, bool clockwise);
    
public:
    RubiksCube(float cubeSize = 0.3f, float spacing = 0.01f); 
    inline const std::vector<Cubie>& GetCubies() const { return m_Cubies; }
    Cubie& GetCubie(int shapeIndex);   
    std::vector<int> GetCubiesInLayerByCurrentPos(int layer, char axis);     
    glm::mat4 GetModelMatrix(int shapeIndex) const;   
    void UpdateCubie(int shapeIndex, const glm::vec3& position, const glm::mat4& rotation);

    void TranslateCubie(int shapeIndex, const glm::vec3& translation);
    void RotateCubie(int shapeIndex, const glm::mat4& rotationDelta);
    
    void RotateRightWall();
    void RotateLeftWall();
    void RotateUpWall();
    void RotateDownWall();
    void RotateBackWall();
    void RotateFrontWall();
    
    void FlipRotationDirection();
    void DivideRotationAngle();
    void MultiplyRotationAngle();
    
    inline float GetCubeSize() const { return m_CubeSize; }
    inline float GetSpacing() const { return m_Spacing; }
    inline float GetRotationAngle() const { return m_RotationAngle; }
    inline bool IsClockwise() const { return m_Clockwise; }
};

