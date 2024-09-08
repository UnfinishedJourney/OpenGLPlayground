#include "Scene.h"

Scene::Scene()
{
	m_Objs.push_back({ std::make_unique<Model>("../assets/rubber_duck/scene.gltf", "../shaders/Light.shader") });
	Light l;
	l.m_Position = { 0.0, 0.0, 0.0, 1.0 };
	l.m_Intensity = 1.0;
	m_Lights.push_back(l);

	std::unique_ptr<Shader> shader = std::make_unique<Shader>("../shaders/Light.shader");
	shader->Bind();
	shader->SetUniform4f("u_L_pos", l.m_Position[0], l.m_Position[1], l.m_Position[2], l.m_Position[3]);
	shader->SetUniformf("u_L_intensity", l.m_Intensity);
	m_Objs[0]->AddShader(std::move(shader));
}

void Scene::Draw()
{
	for (auto& m : m_Objs)
	{
		m->Draw();
	}
}
