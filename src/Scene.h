#pragma once
#include <vector>
#include <memory>
#include "Lights.h"
#include "Model.h"
#include "Shader.h"

class Scene
{
private:
	std::vector<Light> m_Lights;
	std::vector<std::unique_ptr<Model>> m_Objs;

public:
	Scene();
	virtual ~Scene()
	{}

	virtual void Render();
	virtual void InitScene();
	virtual void Update(float dt);

	inline void AddModel(std::unique_ptr<Model> model)
	{
		m_Objs.push_back(std::move(model));
	}

	inline void AddLight(Light light)
	{
		m_Lights.push_back(light);
	}
};