// transform.hpp

#pragma once

#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class cTransform
    {
    public:
        explicit cTransform() = default;
        ~cTransform() = default;

        void Transform();

        inline const cVector3& GetPosition() const { return _position; }
        inline const cVector3& GetRotation() const { return _rotation; }
        inline const cVector3& GetScale() const { return _scale; }
        inline const cMatrix4& GetWorld() const { return _world; }

    private:
        cVector3 _position = cVector3(0.0f);
        cVector3 _rotation = cVector3(0.0f);
        cVector3 _scale = cVector3(1.0f);
        cMatrix4 _world = cMatrix4(1.0f);
    };
}