/**
*@file GameOverScene.cpp
*/

#include "GLFWEW.h"
#include "GameOverScene.h"
#include "TitleScene.h"

/**
*�V�[��������������.
*
*@retval true	����������
*@retval false	���������s.�Q�[���i�s�s�ɂ��A�v���O�������I�����邱��
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
*�v���C���[�̓��͂���������.
*/

void GameOverScene::ProcessInput() {

	GLFWEW::Window& window = GLFWEW::Window::Instance();

	if (window.GetGamePad().buttonDown & GamePad::START) {
		bgm->Stop();
		SceneStack::Instance().Replace(std::make_shared<TitleScene>());

	}
}

/**
*�V�[�����X�V����.
*
*@param deltaTime	�O�񂩂�̍X�V����̌o�ߎ��ԁi�b�j.
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
	fontRenderer.AddString(glm::vec2(-100, 0), L"�Q�[���I�[�o�[");
	fontRenderer.AddString(glm::vec2(-250, -100), L"Enter�������ƃ^�C�g���ɖ߂�܂�");
	fontRenderer.EndUpdate();
}

/*
*�V�[����`�悷��.
*/
void GameOverScene::Render() {

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	spriteRenderer.Draw(screenSize);
	fontRenderer.Draw(screenSize);
}

/**
* �V�[����j������.
*/
void GameOverScene::Finalize() {

	spriteRenderer.Clear();
}