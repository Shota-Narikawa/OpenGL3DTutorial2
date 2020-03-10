/**
*@file StatusScene.cpp
*/

#include "GLFWEW.h"
#include "StatusScene.h"
#include "MainGameScene.h"
#include "TitleScene.h"
#include "Font.h"

/**
*�V�[��������������.
*
*@retval true	����������.
*@retval false	���������s.�Q�[���i�s�s�ɂ��A�v���O�������I�����邱��.
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
*�v���C���[�̓��͂���������.
*/

void StatusScene::ProcessInput() {

	GLFWEW::Window& window = GLFWEW::Window::Instance();
	

	if (window.GetGamePad().buttonDown & GamePad::START) {

		SceneStack::Instance().Pop();
	}
}

/**
*�V�[�����X�V����.
*
*@param deltaTime	�O��̍X�V����̌o�ߎ���.
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
	/*fontRenderer.AddString(glm::vec2(-w * 0.5f + 32, h * 0.5f - lineHeight), L"�X�e�[�^�X���");*/
	fontRenderer.Color(glm::vec4(0.1f, 0.1f, 0.1f, 1));

	fontRenderer.AddString(glm::vec2(-530, 320), L"�X�e�[�^�X�ڍ�");
	fontRenderer.AddString(glm::vec2(-530, 270), L"�v���C���[(�O���@�t)");
	fontRenderer.AddString(glm::vec2(-150, 200), L"Lv:");
	fontRenderer.AddString(glm::vec2(-175, 100), L"�X�L���F");
	fontRenderer.AddString(glm::vec2(-240, 0), L"�W�����v�́F");
	fontRenderer.AddString(glm::vec2(-210, -100), L"�X�s�[�h�F");
	fontRenderer.AddString(glm::vec2(-20, -200), L"�Q�[���ɖ߂�");
	fontRenderer.AddString(glm::vec2(-20, -300), L"�^�C�g���ɖ߂�");

	fontRenderer.AddString(glm::vec2(250, 280), L"�R�}���h�ꗗ");
	fontRenderer.AddString(glm::vec2(250, 200), L"WSAD:�ړ�");
	fontRenderer.AddString(glm::vec2(250, 120), L"J�F�W�����v");
	fontRenderer.AddString(glm::vec2(250, 40), L"K�F�U��");
	fontRenderer.AddString(glm::vec2(250, -40), L"I�F�J��������");
	fontRenderer.AddString(glm::vec2(250, -120), L"Enter:�X�e�[�^�X���");

	fontRenderer.EndUpdate();
}

/**
*�V�[����`�悷��.
*/

void StatusScene::Render() {

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	spriteRenderer.Draw(screenSize);
	fontRenderer.Draw(screenSize);

}

/**
* �V�[����j������.
*/
void StatusScene::Finalize()
{
}