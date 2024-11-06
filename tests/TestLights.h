//#pragma once
//
//#include "Test.h"
//#include "Renderer/RenderObject.h"
//#include "Resources/ResourceManager.h"
//
//#include <GLFW/glfw3.h>
//#include <memory>
//namespace test {
//
//	class LightObject : public RenderObject {
//	public:
//		LightObject(const std::shared_ptr<Mesh>& mesh,
//			const MeshLayout& meshLayout,
//			const std::string& materialName,
//			const std::string& shaderName,
//			const std::shared_ptr<Transform>& transform)
//			: RenderObject(mesh, meshLayout, materialName, shaderName, transform)
//		{}
//	};
//
//	class TestLights : public Test
//	{
//	public:
//		TestLights(std::shared_ptr<Renderer>& renderer);
//		//~TestLight() override = default;
//
//		void OnUpdate(float deltaTime) override;
//		void OnRender() override;
//		void OnImGuiRender() override;
//	private:
//		std::vector<std::shared_ptr<LightObject>> m_Pig;
//	};
//}


#pragma once

#include "Test.h"
#include "Renderer/RenderObject.h"
#include <memory>

namespace test {

    class TestLights : public Test
    {
    public:
        TestLights();
        ~TestLights() override = default;

        void OnEnter() override;
        void OnExit() override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnImGuiRender() override;

    private:
        std::shared_ptr<RenderObject> m_Cube;
    };

}