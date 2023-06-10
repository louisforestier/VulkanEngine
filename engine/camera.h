#pragma once

#include <glm/glm.hpp>

class Transform;

class Camera
{

protected:
    float _nearPlane;
    float _farPlane;
    float _viewportHeight;
    float _viewportWidth;
public:
    Camera(float height, float width, float nearPlane, float farPlane);
    virtual ~Camera() = default;
    virtual glm::mat4 get_projection_matrix() = 0;
    glm::mat4 get_view_matrix(const Transform& transform);
};

class OrthographicCamera : public Camera
{
public:
    OrthographicCamera(float height, float width, float nearPlane, float farPlane);
    glm::mat4 get_projection_matrix();
};

class PerspectiveCamera : public Camera 
{
private:
    float _fov;
public:
    PerspectiveCamera(float fov, float height, float width, float nearPlane, float farPlane);
    glm::mat4 get_projection_matrix();
};


