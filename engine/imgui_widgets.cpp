#include "imgui_widgets.h"
#include "transform.h"

void TransformWidget(Transform& transform)
{
    ImGui::Begin("Transform");
    Vec3Widget("Position", transform.getPosition(),[&transform](const glm::vec3& vec){transform.setPosition(vec);});
    Vec3Widget("Rotation", transform.getRotation(),[&transform](const glm::vec3&  vec){transform.setRotation(glm::radians(vec));});
    Vec3Widget("Scale", transform.getScale(),[&transform](const glm::vec3& vec){transform.setScale(vec);});
    ImGui::End();						
}
 