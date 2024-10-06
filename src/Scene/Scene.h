//#pragma once
//#include "Scene/Lights.h"
//#include "Renderer/RenderObject.h"
//#include "Graphics/Shaders/Shader.h"
//
//#include <vector>
//#include <memory>
//#include <algorithm>
//
//class Scene
//{
//public:
//	Scene() = default;
//	~Scene()
//	{}
//
//	void BindLights();
//	void Render();
//	void Update(float dt);
//	void AddLights(std::vector<Light> lights)
//	{
//		std::copy(lights.begin(), lights.end(), std::back_inserter(m_Lights));
//	}
//	void AddLight(Light light)
//	{
//		m_Lights.push_back(light);
//	}
//	void AddObj(std::unique_ptr<RenderObject> obj)
//	{
//		m_Objs.push_back(std::move(obj));
//	}
//	void AddLightShader(std::shared_ptr<Shader> lightShader)
//	{
//		m_LightShader = lightShader;
//	}
//
//protected:
//	std::vector<Light> m_Lights;
//	std::vector<std::unique_ptr<RenderObject>> m_Objs;
//	std::shared_ptr<Shader> m_LightShader;
//};