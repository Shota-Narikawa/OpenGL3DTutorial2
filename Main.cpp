// OpenGL3DTutorial2.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "Src\TitleScene.h"
#include "Src\GLFWEW.h"
#include "stdafx.h"


int main()
{
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	window.Init(1280, 760, "アクションゲーム");

	SceneStack& sceneStack = SceneStack::Instance();
	sceneStack.Push(std::make_shared<TitleScene>());

	while (!window.ShouldClose()) {

		const float deltaTime = window.DeltaTime();
		
		window.UpDateTimer();
		sceneStack.Update(deltaTime);
		sceneStack.Render();
		window.SwapBuffers();
	}

    return 0;
}

