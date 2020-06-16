/**
*@file GameOverScene.cpp
*/

#include "GLFWEW.h"
#include "GameOverScene.h"
#include "TitleScene.h"

/**
*シーンを初期化する.
*
*@retval true	初期化成功
*@retval false	初期化失敗.ゲーム進行不可につき、プログラムを終了すること
*/
bool GameOverScene::Initialize() {

	spriteRenderer.Init(1000, "Res/Sprite.vert", "Res/Sprite.frag");
	sprites.reserve(100);
	Sprite spr(Texture::Image2D::Create("Res/GameOver.tga"));
	spr.Scale(glm::vec2(2));
	sprites.push_back(spr);

	bgm = Audio::Engine::Instance().Prepare("Res/Audio/TitleScene.mp3");
	bgm->Play(Audio::Flag_Loop);

	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/font.fnt");

	return true;
}

/**
*プレイヤーの入力を処理する.
*/

void GameOverScene::ProcessInput() {

	GLFWEW::Window& window = GLFWEW::Window::Instance();

	if (window.GetGamePad().buttonDown & GamePad::START) {
		bgm->Stop();
		SceneStack::Instance().Replace(std::make_shared<TitleScene>());

	}
}

/**
*シーンを更新する.
*
*@param deltaTime	前回からの更新からの経過時間（秒）.
*/
void GameOverScene::Update(float deltaTime) {

	spriteRenderer.BeginUpdate();
	for (const Sprite& e : sprites) {
		spriteRenderer.AddVertices(e);
	}
	spriteRenderer.EndUpdate();

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const float w = window.Width();
	const float h = window.Height();
	const float lineHeight = fontRenderer.LineHeight();

	fontRenderer.BeginUpdate();
	fontRenderer.Color(glm::vec4(1, 0.2f, 0.1f, 1));
	fontRenderer.AddString(glm::vec2(-100, 0), L"ゲームオーバー");
	fontRenderer.AddString(glm::vec2(-250, -100), L"Enterを押すとタイトルに戻ります");
	fontRenderer.EndUpdate();
}

/*
*シーンを描画する.
*/
void GameOverScene::Render() {

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	spriteRenderer.Draw(screenSize);
	fontRenderer.Draw(screenSize);
}

/**
* シーンを破棄する.
*/
void GameOverScene::Finalize() {

	spriteRenderer.Clear();
}