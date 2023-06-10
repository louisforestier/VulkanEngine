#include "camera.h"
#include <glm/gtx/transform.hpp>
#include "transform.h"

Camera::Camera(float height, float width, float nearPlane, float farPlane)
: _viewportHeight(height)
, _viewportWidth(width)
, _nearPlane(nearPlane)
, _farPlane(farPlane)
{
}

glm::mat4 Camera::get_view_matrix(const Transform &transform)
{
    return glm::inverse(transform.get_matrix());
}

OrthographicCamera::OrthographicCamera(float height, float width, float nearPlane, float farPlane)
: Camera(height, width, nearPlane, farPlane)
{
}

glm::mat4 OrthographicCamera::get_projection_matrix()
{
    return glm::ortho(0.f, _viewportWidth, 0.f, _viewportHeight, _nearPlane, _farPlane);
}

PerspectiveCamera::PerspectiveCamera(float fov, float height, float width, float nearPlane, float farPlane)
: Camera(height, width, nearPlane, farPlane)
, _fov(fov)
{
}

glm::mat4 PerspectiveCamera::get_projection_matrix()
{
    return glm::perspective(glm::radians(_fov),_viewportWidth/_viewportHeight ,_nearPlane, _farPlane);
}

