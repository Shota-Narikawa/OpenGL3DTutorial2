/**
*@file StatusScene.cpp
*/

#include "GLFWEW.h"
#include "StatusScene.h"
#include "MainGameScene.h"
#include "TitleScene.h"
#include "Font.h"

/**
*シーンを初期化する.
*
*@retval true	初期化成功.
*@retval false	初期化失敗.ゲーム進行不可につき、プログラムを終了すること.
*/

bool StatusScene::Initialize() {

	GLFWEW::Window& window = GLFWEW::Window::Instance();

	spriteRenderer.Init(1000, "Res/Sprite.vert", "Res/Sprite.frag");
	sprites.reserve(100);
	Sprite spr(Texture::Image2D::Create("Res/statusScene.tga"));
	spr.Scale(glm::vec2(2));
	sprites.push_back(spr);

	Sprite spr2(Texture::Image2D::Create("Res/player.tga"));
	spr2.Position(glm::vec3(-400, -50, 0));
	spr2.Scale(glm::vec2(1.5f));
	sprites.push_back(spr2);

	Sprite spr3(Texture::Image2D::Create("Res/select.tga"));
	spr3.Position(glm::vec3(-80, -190, 0));
	spr3.Scale(glm::vec2(0.08f));
	sprites.push_back(spr3);

	for (int i = 0; i < 10; ++i) {
		const std::string filename =
			std::string("Res/CNumber_") + (char)('0' + i) + ".tga";
		texCNumber[i].Reset(Texture::LoadImage2D(filename.c_str()));
	}

	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/font.fnt");

	return true;
}

/**
*プレイヤーの入力を処理する.
*/

void StatusScene::ProcessInput() {

	GLFWEW::Window& window = GLFWEW::Window::Instance();
	

	if (window.GetGamePad().buttonDown & GamePad::START) {

		SceneStack::Instance().Pop();
	}
}

/**
*シーンを更新する.
*
*@param deltaTime	前回の更新からの経過時間.
*/

void StatusScene::Update(float deltaTime) {

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
	/*fontRenderer.AddString(glm::vec2(-w * 0.5f + 32, h * 0.5f - lineHeight), L"ステータス画面");*/
	fontRenderer.Color(glm::vec4(0.1f, 0.1f, 0.1f, 1));

	fontRenderer.AddString(glm::vec2(-530, 320), L"ステータス詳細");
	fontRenderer.AddString(glm::vec2(-530, 270), L"プレイヤー(三蔵法師)");
	fontRenderer.AddString(glm::vec2(-150, 200), L"Lv:");
	fontRenderer.AddString(glm::vec2(-175, 100), L"スキル：");
	fontRenderer.AddString(glm::vec2(-240, 0), L"ジャンプ力：");
	fontRenderer.AddString(glm::vec2(-210, -100), L"スピード：");
	fontRenderer.AddString(glm::vec2(-20, -200), L"ゲームに戻る");
	fontRenderer.AddString(glm::vec2(-20, -300), L"タイトルに戻る");

	fontRenderer.AddString(glm::vec2(250, 280), L"コマンド一覧");
	fontRenderer.AddString(glm::vec2(250, 200), L"WSAD:移動");
	fontRenderer.AddString(glm::vec2(250, 120), L"J：ジャンプ");
	fontRenderer.AddString(glm::vec2(250, 40), L"K：攻撃");
	fontRenderer.AddString(glm::vec2(250, -40), L"I：カメラ操作");
	fontRenderer.AddString(glm::vec2(250, -120), L"Enter:ステータス画面");

	fontRenderer.EndUpdate();
}

/**
*シーンを描画する.
*/

void StatusScene::Render() {

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	spriteRenderer.Draw(screenSize);
	fontRenderer.Draw(screenSize);

}

/**
* シーンを破棄する.
*/
void StatusScene::Finalize()
{
}