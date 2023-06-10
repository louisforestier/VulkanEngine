#include "transform.h"

Transform::Transform()
: _pos(0.f, 0.f, 0.f)
, _orientation(glm::angleAxis(0.f, glm::vec3{0.f,0.f,1.0f}))
, _eulerAngles()
, _scale(1.f, 1.f, 1.f)
{
}

Transform::~Transform()
{
}

const glm::mat4& Transform::get_matrix() const
{
    return _matrix;
}

void Transform::update()
{
    _matrix = glm::translate(glm::scale(glm::mat4(1.f), _scale), _pos) * glm::mat4_cast(_orientation);
}

void Transform::translate(const glm::vec3 &translation)
{
    _pos += translation;
}

void Transform::rotate(const glm::vec3 &eulers)
{
    _orientation *= glm::quat(glm::radians(eulers));
    _eulerAngles += eulers;
}

void Transform::rotate(float angle, const glm::vec3 &axis)
{
    _orientation *= glm::angleAxis(glm::radians(angle), axis);
}

void Transform::yaw(float angle) 
{
    rotate(angle, {0.f, 1.f, 0.f});
    _eulerAngles.y += angle;
}

void Transform::pitch(float angle)
{ 
    rotate(angle, {1.f, 0.f, 0.f}); 
    _eulerAngles.x += angle;
}

void Transform::roll(float angle)
{
    rotate(angle, {0.f, 0.f, 1.f}); 
    _eulerAngles.z += angle;
}

