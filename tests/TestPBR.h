#pragma once

#include "Test.h"
#include "ResourceManager.h"
#include "RenderObject.h"
#include "Scene.h"

#include <GLFW/glfw3.h>
#include <memory>

namespace test {

	class PBRObject : public RenderObject {
	public:
		PBRObject(std::vector<std::shared_ptr<MeshBuffer>> meshBuffers, std::shared_ptr<Material> mat, std::unique_ptr<Transform> transform)
			: RenderObject(meshBuffers, mat, std::move(transform))
		{}
		virtual void Render() override;
	};

	class TestPBR : public Test
	{
	public:
		TestPBR();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<Scene> m_Scene;
		std::unique_ptr<ResourceManager> m_ResourceManager;
	};
}