#include "Scene.h"
#include <GLFW/glfw3.h>
#include "gtc/constants.hpp"

#include <sstream>

void Scene::BindLights()
{
	if (m_Lights.size() == 0 || !m_LightShader)
		return;

	m_LightShader->Bind();

	size_t nLights = m_Lights.size();

	for (size_t i = 0; i < nLights; i++)
	{
		std::stringstream name;
		name << "lights[" << i << "].Position";
		m_LightShader->SetUniform(name.str(), FrameData::s_View * m_Lights[i].position);
		name.str("");
		name << "lights[" << i << "].Color";
		m_LightShader->SetUniform(name.str(), m_Lights[i].color);
	}
}

void Scene::Render()
{
	BindLights();
	for (auto& m : m_Objs)
	{
		m->Render();
	}
}

void Scene::Update(float dt)
{
	for (auto& m : m_Objs)
	{
		m->Update(dt);
	}
}

