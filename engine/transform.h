#pragma once

#include <glm/ext.hpp>

class Transform
{
private:
    
    glm::vec3 _pos;
    glm::quat _orientation;
    glm::vec3 _eulerAngles;
    glm::vec3 _scale;
    glm::mat4 _matrix;
public:
    Transform();
    ~Transform();
    const glm::mat4& get_matrix() const;
    void update();
    void translate(const glm::vec3& translation);
    void rotate(const glm::vec3& eulers);
    void rotate(float angle, const glm::vec3& axis);
    void yaw(float angle);
    void pitch(float angle);
    void roll(float angle);
    const glm::quat& getOrientation(){return _orientation;}
    const glm::vec3& getPosition(){return _pos;}
    const glm::vec3& getRotation(){return _eulerAngles;}
    const glm::vec3& getScale(){return _scale;}
    void setPosition(const glm::vec3& pos){_pos = pos;}
    void setRotation(const glm::vec3& eulerAngles)
    {
        _eulerAngles = eulerAngles;
        _orientation = glm::quat(eulerAngles);
    }
    void setScale(const glm::vec3& scale){_scale = scale;}

};
