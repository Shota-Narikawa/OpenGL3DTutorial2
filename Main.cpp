// OpenGL3DTutorial2.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include <Windows.h>
#include "Src\TitleScene.h"
#include "Src\GLFWEW.h"
#include "Src\SkeletalMesh.h"
#include "Src\EventScript.h"
#include "stdafx.h"


int main()
{
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	if (!window.Init(1280, 760, "OpenGL 3D 2019")) {

		return 1;
	}

	//音声再生プログラムを初期化する.
	Audio::Engine& audioEngine = Audio::Engine::Instance();
	if (!audioEngine.Initialize()) {
		return 1;

	}

	//スケルタル・アニメーションを利用可能にする.
	Mesh::SkeletalAnimation::Initialize();

	SceneStack& sceneStack = SceneStack::Instance();
	sceneStack.Push(std::make_shared<TitleScene>());

	EventScriptEngine& scriptEngine = EventScriptEngine::Instance();
	scriptEngine.Init();

	window.InitTimer();
	while (!window.ShouoldClose()) {

		window.UpdateTimer();

		//スケルタル・アニメーション用データの作成準備.
		Mesh::SkeletalAnimation::ResetUniformData();

		//ESCキーが押されていたら終了ウィンドウを表示.
		if (window.IsKeyPressed(GLFW_KEY_ESCAPE)) {

			if (MessageBox(nullptr, "ゲームを終了しますか？", "終了", MB_OKCANCEL) == IDOK) {
				break;
			}
		}

		const float deltaTime = window.DeltaTime();
		sceneStack.Update(deltaTime);
		scriptEngine.Update(deltaTime);

		//スケルタル・アニメーション用データをGPUメモリに転送.
		Mesh::SkeletalAnimation::UploadUniformData();

		//音声再生プログラムを更新する.
		audioEngine.Update();

		//バックバッファを消去する.
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//GLコンテキストのパラメーターを設定.
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		sceneStack.Render();
		scriptEngine.Draw();
		window.SwapBuffers();
	}

	//スケルタル・アニメーションの利用を終了する.
	Mesh::SkeletalAnimation::Finalize();

	//音声再生プログラムを終了する.
	audioEngine.Finalize();

	return 0;
}

