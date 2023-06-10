#include "fly_animator.h"

FlyAnimator::FlyAnimator(Transform &transform)
: _transform(transform)
{
}

void FlyAnimator::update(float deltaTime)
{
    _velocity = glm::vec3();
    if (isPressed(SDL_SCANCODE_W))
    {
        _velocity.z -= 1.f;
    }
    if (isReleased(SDL_SCANCODE_W))
    {
        _velocity.z += 1.f;
    }
    if (isPressed(SDL_SCANCODE_S))
    {
        _velocity.z += 1.f;
    }
    if (isReleased(SDL_SCANCODE_S))
    {
        _velocity.z -= 1.f;
    }
    if (isPressed(SDL_SCANCODE_A))
    {
        _velocity.x -= 1.f;
    }
    if (isReleased(SDL_SCANCODE_A))
    {
        _velocity.x += 1.f;
    }
    if (isPressed(SDL_SCANCODE_D))
    {
        _velocity.x += 1.f;
    }
    if (isReleased(SDL_SCANCODE_D))
    {
        _velocity.x -= 1.f;
    }
    if (isPressed(SDL_SCANCODE_LSHIFT))
    {
        _sprinting = true;
    }
    if (isReleased(SDL_SCANCODE_LSHIFT))
    {
        _sprinting = false;
    }
    float yaw = 0.f;
    float pitch = 0.f;
    float roll = 0.f;
    if (isPressed(SDL_SCANCODE_Q))
    {
        roll = 0.5f;
    }
    if (isPressed(SDL_SCANCODE_E))
    {
        roll = -0.5f;
    }

    if (isPressed(SDL_BUTTON_LEFT))
    {					
        yaw = -_xrel * 0.03f * deltaTime;
	    pitch = -_yrel * 0.03f * deltaTime;
    }
    _transform.rotate({pitch, yaw,roll});
    
    _xrel = 0.f;
    _yrel = 0.f;
    const float cam_vel = 0.2f * deltaTime * (0.01f + _sprinting * 0.05f);
	glm::vec3 forward = { 0,0,cam_vel };
	glm::vec3 right = { cam_vel,0,0 };
	glm::vec3 up = { 0,cam_vel,0 };

	glm::quat rot = _transform.getOrientation();

	forward = rot * forward;
	right = rot * right;
	up = rot * up;

	_velocity = _velocity.z * forward + _velocity.x * right + _velocity.y * up;

	_transform.translate(_velocity);
}
