#include "transformComponent.h"

void TransformComponent::SetPosition(const glm::vec3& pos)
{
    position = pos;
    transformDirty = true;
}

void TransformComponent::SetRotation(const glm::quat& rot)
{
    rotation = rot;
    transformDirty = true;
}

void TransformComponent::SetScale(const glm::vec3& s)
{
    scale = s;
    transformDirty = true;
}