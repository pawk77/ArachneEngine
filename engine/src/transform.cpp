// transform.cpp

#include "transform.hpp"

using namespace types;

namespace triton
{
    void cTransform::Transform()
    {
        const quaternion quatX = glm::angleAxis(_rotation.x, vector3(1.0f, 0.0f, 0.0f));
        const quaternion quatY = glm::angleAxis(_rotation.y, vector3(0.0f, 1.0f, 0.0f));
        const quaternion quatZ = glm::angleAxis(_rotation.z, vector3(0.0f, 0.0f, 1.0f));
        _world = glm::translate(matrix4(1.0f), _position) * glm::toMat4(quatZ * quatY * quatX) * glm::scale(matrix4(1.0f), _scale);
    }
}