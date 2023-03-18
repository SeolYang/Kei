#pragma once
#include <PCH.h>

namespace sy::component
{
struct TransformComponent : ecs::Component
{
    glm::vec3 Position;
    glm::vec3 Scale;
    glm::quat Rotation;
};

inline glm::mat4 TransformToMatrix(const TransformComponent& transform)
{
    /** SRT */
    return glm::scale(transform.Scale) * (glm::toMat4(transform.Rotation) * glm::translate(transform.Position));
}
} // namespace sy::component
