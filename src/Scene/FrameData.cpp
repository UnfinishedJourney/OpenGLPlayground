#include "Scene/FrameData.h"

int Screen::s_Width = 800;
int Screen::s_Height = 600;

float Screen::s_DisplayHeight = 13.23f;     // inches
float Screen::s_ViewerDistance = 24.0f;

glm::mat4 FrameData::s_View = glm::mat4(1.0f);
glm::mat4 FrameData::s_Projection = glm::mat4(1.0f);
glm::vec3 FrameData::s_CameraPos = glm::vec3(0.0f);