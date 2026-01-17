#include "RubiksCube.h"
#include <algorithm>
#include <cmath>

RubiksCube::RubiksCube(float cubeSize, float spacing) 
    : m_CubeSize(cubeSize), m_Spacing(spacing), m_RotationAngle(90.0f), m_Clockwise(true)
{
    // Grid positions range from -1 to 1 in each axis
    int index = 0;
    for (int z = -1; z <= 1; z++) {
        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                glm::vec3 gridPos(x, y, z);
                glm::vec3 worldPos = gridPos * (cubeSize + spacing);
                Cubie cubie(index, gridPos);
                cubie.position = worldPos;
                cubie.translation = glm::translate(glm::mat4(1.0f), worldPos);
                cubie.rotation = glm::mat4(1.0f);
                m_Cubies.push_back(cubie);
                index++;
            }
        }
    }
}

Cubie& RubiksCube::GetCubie(int shapeIndex) {
    return m_Cubies[shapeIndex];
}

std::vector<int> RubiksCube::GetCubiesInLayerByCurrentPos(int layer, char axis) {
    std::vector<int> result;
    
    // Calculate the target position for this layer
    float targetPos = layer * (m_CubeSize + m_Spacing);
    float threshold = (m_CubeSize + m_Spacing) * 0.1f;  // Small threshold for floating point comparison
    
    for (int i = 0; i < 27; i++) {
        const Cubie& cubie = m_Cubies[i];
        bool inLayer = false;
        float currentPos;
        
        switch (axis) {
            case 'x':
                currentPos = cubie.position.x;
                break;
            case 'y':
                currentPos = cubie.position.y;
                break;
            case 'z':
                currentPos = cubie.position.z;
                break;
            default:
                continue;
        }
        
        // Check if cubie's current position is in this layer within threshold
        inLayer = (std::abs(currentPos - targetPos) < threshold);
        
        if (inLayer) {
            result.push_back(i);
        }
    }
    
    return result;
}

glm::mat4 RubiksCube::GetModelMatrix(int shapeIndex) const {
    if (shapeIndex < 0 || shapeIndex >= 27) {
        return glm::mat4(1.0f);
    }
    
    const Cubie& cubie = m_Cubies[shapeIndex];
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(m_CubeSize));
    return cubie.translation * cubie.rotation * scale;
}

void RubiksCube::UpdateCubie(int shapeIndex, const glm::vec3& position, const glm::mat4& rotation) {
    if (shapeIndex < 0 || shapeIndex >= 27) {
        return;
    }
    
    Cubie& cubie = m_Cubies[shapeIndex];
    cubie.position = position;
    cubie.rotation = rotation;
    cubie.translation = glm::translate(glm::mat4(1.0f), position);
}

void RubiksCube::TranslateCubie(int shapeIndex, const glm::vec3& translation) {
    if (shapeIndex < 0 || shapeIndex >= 27) {
        return;
    }
    
    Cubie& cubie = m_Cubies[shapeIndex];
    cubie.position += translation;
    cubie.translation = glm::translate(glm::mat4(1.0f), cubie.position);
}

void RubiksCube::RotateCubie(int shapeIndex, const glm::mat4& rotationDelta) {
    if (shapeIndex < 0 || shapeIndex >= 27) {
        return;
    }
    
    Cubie& cubie = m_Cubies[shapeIndex];
    cubie.rotation = rotationDelta * cubie.rotation;
}

void RubiksCube::FlipRotationDirection() {
    m_Clockwise = !m_Clockwise;
}

void RubiksCube::DivideRotationAngle() {
    m_RotationAngle /= 2.0f;
    if (m_RotationAngle < 90.0f) {
        m_RotationAngle = 90.0f;
    }
}

void RubiksCube::MultiplyRotationAngle() {
    m_RotationAngle *= 2.0f;
    if (m_RotationAngle > 180.0f) {
        m_RotationAngle = 180.0f;
    }
}

void RubiksCube::RotateLayer(int layer, char axis, float angle, bool clockwise) {
    std::vector<int> cubieIndices = GetCubiesInLayerByCurrentPos(layer, axis);
    
    if (cubieIndices.size() != 9) {
        return;
    }
    
    glm::vec3 rotationAxis;
    
    switch (axis) {
        case 'x':
            rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
            break;
        case 'y':
            rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
            break;
        case 'z':
            rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
            break;
    }
    
    float angleRad = glm::radians(clockwise ? angle : -angle);
    
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angleRad, rotationAxis);
    
    glm::vec3 cubeCenter(0.0f, 0.0f, 0.0f);
    
    for (int cubieIdx : cubieIndices) {
        Cubie& cubie = GetCubie(cubieIdx);
        
        glm::vec3 relativePos = cubie.position - cubeCenter;
        glm::vec4 rotatedPos = rotationMatrix * glm::vec4(relativePos, 1.0f);
        glm::vec3 newPosition = cubeCenter + glm::vec3(rotatedPos);
        
        cubie.rotation = rotationMatrix * cubie.rotation;
        cubie.position = newPosition;
        cubie.translation = glm::translate(glm::mat4(1.0f), newPosition);
    }
}   

void RubiksCube::RotateRightWall() {
    RotateLayer(1, 'x', m_RotationAngle, m_Clockwise);
}

void RubiksCube::RotateLeftWall() {
    RotateLayer(-1, 'x', m_RotationAngle, m_Clockwise);
}

void RubiksCube::RotateUpWall() {
    RotateLayer(1, 'y', m_RotationAngle, m_Clockwise);
}

void RubiksCube::RotateDownWall() {
    RotateLayer(-1, 'y', m_RotationAngle, m_Clockwise);
}

void RubiksCube::RotateBackWall() {
    RotateLayer(-1, 'z', m_RotationAngle, m_Clockwise);
}

void RubiksCube::RotateFrontWall() {
    RotateLayer(1, 'z', m_RotationAngle, m_Clockwise);
}

