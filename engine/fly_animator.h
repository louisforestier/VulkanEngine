#pragma once
#include "event_handler.h"
#include "transform.h"

class FlyAnimator : public SDLEventHandler
{
private:
    glm::vec3 _velocity;
    bool _sprinting;
    Transform& _transform;
public:
    FlyAnimator(Transform& transform);
    ~FlyAnimator() = default;

    void update(float deltaTime) override;
};
