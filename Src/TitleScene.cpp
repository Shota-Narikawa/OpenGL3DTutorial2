/**
*@file TitleScene.cpp
*/
#include "TitleScene.h"
#include "MainGameScene.h"
#include "Sprite.h"
#include "PlayerActor.h"
#include <Windows.h>
#include "GLFWEW.h"

/**
*�V�[��������������.
*
*@retval true	����������.
*@retval false	���������s.�Q�[���i�s�s�ɂ��A�v���O�������I�����邱��.
*/

bool TitleScene::Initialize() {

	spriteRenderer.Init(1000, "Res/Sprite.vert", "Res/Sprite.frag");
	sprites.reserve(100);

	Sprite spr0(Texture::Image2D::Create("Res/TitleBack2.tga"));
	spr0.Scale(glm::vec2(1));
	sprites.push_back(spr0);

	Sprite spr1(Texture::Image2D::Create("Res/MenuWindow5.tga"));
	spr1.Scale(glm::vec2(0));
	sprites.push_back(spr1);

	Sprite IconBlue2(Texture::Image2D::Create("Res/select.tga"));
	IconBlue2.Position(glm::vec3(-380, 210, 0));
	//IconBlue39.Scale(glm::vec2(0.8f , 0.15f));
	IconBlue2.Scale(glm::vec2(0));	//IconBlue36.Scale(glm::vec2(0.28f,0.22f));
	sprites.push_back(IconBlue2);

	//�I���A�C�R��.
	Sprite selectIcon3(Texture::Image2D::Create("Res/selectA.tga"));
	selectIcon3.Position(glm::vec3(-490, 210, 0));
	selectIcon3.Scale(glm::vec2(0));
	sprites.push_back(selectIcon3);

	//�I���A�C�R��.
	Sprite selectIcon4(Texture::Image2D::Create("Res/selectA.tga"));
	selectIcon4.Position(glm::vec3(-490, 210, 0));
	selectIcon4.Scale(glm::vec2(0));
	sprites.push_back(selectIcon4);

	Sprite GamePad5(Texture::Image2D::Create("Res/GamePad2.tga"));
	GamePad5.Scale(glm::vec2(0));
	sprites.push_back(GamePad5);

	Sprite KeyBord6(Texture::Image2D::Create("Res/KeyBord.tga"));
	KeyBord6.Scale(glm::vec2(0));
	KeyBord6.Position(glm::vec3(150, 0, 0));
	sprites.push_back(KeyBord6);


	Sprite Title7(Texture::Image2D::Create("Res/TitleLogo.tga"));
	Title7.Scale(glm::vec2(1.7f));
	Title7.Position(glm::vec3(0, 170, 0));
	sprites.push_back(Title7);

	Sprite TitleLogoGS8(Texture::Image2D::Create("Res/GameStart2.tga"));
	TitleLogoGS8.Scale(glm::vec2(1));
	TitleLogoGS8.Position(glm::vec3(0, -50, 0));
	sprites.push_back(TitleLogoGS8);

	Sprite TitleLogoS9(Texture::Image2D::Create("Res/Story.tga"));
	TitleLogoS9.Scale(glm::vec2(1));
	TitleLogoS9.Position(glm::vec3(0, -140, 0));
	sprites.push_back(TitleLogoS9);

	Sprite TitleLogoC10(Texture::Image2D::Create("Res/Credit.tga"));
	TitleLogoC10.Scale(glm::vec2(1));
	TitleLogoC10.Position(glm::vec3(0, -230, 0));
	sprites.push_back(TitleLogoC10);

	Sprite TitleLogoE11(Texture::Image2D::Create("Res/Exit.tga"));
	TitleLogoE11.Scale(glm::vec2(1));
	TitleLogoE11.Position(glm::vec3(0, -320, 0));
	sprites.push_back(TitleLogoE11);

	//BGM���Đ�����.
	bgm = Audio::Engine::Instance().Prepare("Res/Audio/TitleScene2.mp3");
	bgm->Play(Audio::Flag_Loop);

	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/font.fnt");

	return true;
}

/**
*�v���C���[�̓��͂���������.
*/

void TitleScene::ProcessInput() {

	GLFWEW::Window& window = GLFWEW::Window::Instance();

	if (state == State::title) {
		//�I���A�C�R���𓮂���.
		if (window.GetGamePad().buttonDown & GamePad::DPAD_UP) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			if (selectCount >= 1) {
				--selectCount;
				if (selectCount == 0) {
					selectCount = 4;
				}
			}
		}
		//�����
		else if (window.GetGamePad().buttonDown & GamePad::DPAD_DOWN) {
			if (selectCount <= 4) {
				Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
				++selectCount;
				if (selectCount == 5) {
					selectCount = 1;
				}
			}
		}
	}

	if (state == State::tutorial) {
		//�I���A�C�R���𓮂���.
		if (window.GetGamePad().buttonDown & GamePad::DPAD_LEFT) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			if (tutorialCount >= 1) {
				--tutorialCount;
				if (tutorialCount == 0) {
					tutorialCount = 3;
				}
			}
		}
		//�����
		else if (window.GetGamePad().buttonDown & GamePad::DPAD_RIGHT) {
			if (tutorialCount <= 3) {
				Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
				++tutorialCount;
				if (tutorialCount == 4) {
					tutorialCount = 1;
				}
			}
		}
	}

	//����m�F��ʎ�.
	if (state == State::tutorial && window.GetGamePad().buttonDown & GamePad::START) {
		if (tutorialCount == 1) {
			tutorialText = true;
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			sprites[5].Scale(glm::vec2(0.5f));
			sprites[6].Scale(glm::vec2(0));
		}
		else if (tutorialCount == 2) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			tutorialText = false;
			sprites[5].Scale(glm::vec2(0));
			sprites[6].Scale(glm::vec2(0.4f));
		}
		else if (tutorialCount == 3) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			timer = 0.5f;
		}
	}

	//�X�g�[���[���N���W�b�g�m�F��.
	if (state == State::story || state == State::credit) {
		if (window.GetGamePad().buttonDown & GamePad::B) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			state = State::title;
			selectCount = 1;

			sprites[0].Scale(glm::vec2(1));
			sprites[7].Scale(glm::vec2(2));
			sprites[8].Scale(glm::vec2(1));
			sprites[9].Scale(glm::vec2(1));
			sprites[10].Scale(glm::vec2(1));
			sprites[11].Scale(glm::vec2(1));

			sprites[1].Scale(glm::vec2(0));
		}
	}

	//�^�C�g����ʎ�.
	if (state == State::title && window.GetGamePad().buttonDown & GamePad::START) {
		if (selectCount == 1) {

			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			tutorialCount = 1;
			tutorialText = true;
			state = State::tutorial;

			sprites[0].Scale(glm::vec2(0));
			sprites[7].Scale(glm::vec2(0));
			sprites[8].Scale(glm::vec2(0));
			sprites[9].Scale(glm::vec2(0));
			sprites[10].Scale(glm::vec2(0));
			sprites[11].Scale(glm::vec2(0));

			sprites[1].Scale(glm::vec2(1.5f, 4.0f));
			sprites[5].Scale(glm::vec2(0.5f));
		}
		else if (selectCount == 2) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			state = State::story;

			sprites[0].Scale(glm::vec2(0));
			sprites[2].Scale(glm::vec2(0));
			sprites[3].Scale(glm::vec2(0));
			sprites[4].Scale(glm::vec2(0));
			sprites[7].Scale(glm::vec2(0));
			sprites[8].Scale(glm::vec2(0));
			sprites[9].Scale(glm::vec2(0));
			sprites[10].Scale(glm::vec2(0));
			sprites[11].Scale(glm::vec2(0));

			sprites[1].Scale(glm::vec2(1.5f, 4.0f));
		}
		else if (selectCount == 3) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			state = State::credit;

			sprites[0].Scale(glm::vec2(0));
			sprites[2].Scale(glm::vec2(0));
			sprites[3].Scale(glm::vec2(0));
			sprites[4].Scale(glm::vec2(0));
			sprites[7].Scale(glm::vec2(0));
			sprites[8].Scale(glm::vec2(0));
			sprites[9].Scale(glm::vec2(0));
			sprites[10].Scale(glm::vec2(0));
			sprites[11].Scale(glm::vec2(0));

			sprites[1].Scale(glm::vec2(1.5f, 4.0f));
		}
		else if (selectCount == 4) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			exit(0);
		}
	}
}

/**
*�V�[�����X�V����.
*
*@param deltaTime	�O��̍X�V����̌o�ߎ���.
*/

void TitleScene::Update(float deltaTime) {

	spriteRenderer.BeginUpdate();

	for (const Sprite& e : sprites) {

		spriteRenderer.AddVertices(e);
	}
	spriteRenderer.EndUpdate();

	//���[�h��ʒ�.
	if (loadTimer > 0.0f) {
		loadTimer -= deltaTime;
		if (loadTimer <= 0.0f) {
			SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
			return;
		}
	}

	//�V�[���؂�ւ��҂�.
	if (timer > 0.0f) {
		timer -= deltaTime;
		if (timer <= 0.0f) {
			bgm->Stop();
			DeleteSpriteA(sprites, 8);
			state = State::load;
			loadTimer = 2.0f;

			sprites[1].Scale(glm::vec2(1.5f, 4.0f));

			sprites[2].Scale(glm::vec2(0));
			sprites[3].Scale(glm::vec2(0));
			sprites[4].Scale(glm::vec2(0));
			sprites[5].Scale(glm::vec2(0));
			sprites[6].Scale(glm::vec2(0));
		}
	}

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const float w = window.Width();
	const float h = window.Height();
	const float lineHeight = fontRenderer.LineHeight();

	fontRenderer.BeginUpdate();

	if (state == State::tutorial) {

		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 10));
		fontRenderer.AddString(glm::vec2(-280, 260), L"�Q�[�������߂�O�ɑ���̊m�F�����悤");
		fontRenderer.AddString(glm::vec2(-355, -260), L"�Q�[���p�b�h");
		fontRenderer.AddString(glm::vec2(-80, -260), L"�L�[�{�[�h");
		fontRenderer.AddString(glm::vec2(175, -260), L"�{�҂��n�߂�");

		if (tutorialText) {
			fontRenderer.Color(glm::vec4(0.0f, 0.0f, 0.0f, 1));
			fontRenderer.AddString(glm::vec2(-340, 140), L"���_����]");
			fontRenderer.AddString(glm::vec2(-310, 10), L"����");
			fontRenderer.AddString(glm::vec2(-340, -50), L"���j���[");
			fontRenderer.AddString(glm::vec2(-310, -120), L"�ړ�");

			fontRenderer.AddString(glm::vec2(215, 200), L"�W�����v");
			fontRenderer.AddString(glm::vec2(215, 140), L"���_�E��]");
			fontRenderer.AddString(glm::vec2(215, 75), L"�X�L���P");
			fontRenderer.AddString(glm::vec2(215, 10), L"�X�L���Q");
			fontRenderer.AddString(glm::vec2(215, -55), L"�X�L���R");
			fontRenderer.AddString(glm::vec2(215, -115), L"�X�L���S");
		}
		else
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-460, 210), L"WSAD:�ړ�");
			fontRenderer.AddString(glm::vec2(-460, 170), L"L�F�W�����v");
			fontRenderer.AddString(glm::vec2(-460, 130), L"U�F�X�L���P");
			fontRenderer.AddString(glm::vec2(-460, 90), L"H�F�X�L���Q");
			fontRenderer.AddString(glm::vec2(-460, 50), L"K�F�X�L���R");
			fontRenderer.AddString(glm::vec2(-460, 10), L"J�F�X�L���S");
			fontRenderer.AddString(glm::vec2(-460, -30), L"Q�F�J��������]");
			fontRenderer.AddString(glm::vec2(-460, -70), L"E�F�J�����E��]");
			fontRenderer.AddString(glm::vec2(-460, -110), L"SPACE:���j���[");
			fontRenderer.AddString(glm::vec2(-460, -150), L"ENTER:����");
		}
	}

	if (state == State::load) {
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 10));
		fontRenderer.AddString(glm::vec2(-100, 0), L"NowLoading...");
	}

	if (state == State::story) {
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
		fontRenderer.AddString(glm::vec2(-430, 250), L"����͂Ƃ��鐢�E�́A�Ƃ���P�l�̖`���҂̕���ł���B");
		fontRenderer.AddString(glm::vec2(-430, 200), L"������A�`���҂͍���Փx�̃_���W�����֖`���ɏo�Ă����B");
		fontRenderer.AddString(glm::vec2(-430, 150), L"�������A�_���W�����œG�ɔs�k���A�����̐g�ƂȂ��Ă��܂��B");
		fontRenderer.AddString(glm::vec2(-430, 100), L"�G�͐l�Ԃ̍��݂̂���炤�h�\�E���C�[�^�[�h�������B");
		fontRenderer.AddString(glm::vec2(-430, 50), L"���̂ƍ��𕪗����ꂽ���A����钼�O�ɒE�����n��֌��������B");
		fontRenderer.AddString(glm::vec2(-430, 0), L"�����A�n��t�߂ɂ͓G���吨���Ĕ����o�����ɂ��Ȃ��B");
		fontRenderer.AddString(glm::vec2(-430, -50), L"�����؂ꂽ�Ƃ��Ă����̏�Ԃł͒n��ł͐����Ă����Ȃ��B");
		fontRenderer.AddString(glm::vec2(-430, -100), L"�����G�ɒD���Ă��鎩���̑̂����߂������Ȃ��B");
		fontRenderer.AddString(glm::vec2(-430, -150), L"�ނ͍Ăу_���W�����ɐ���A���߂����ӂ������B");
		fontRenderer.AddString(glm::vec2(-430, -200), L"����Ȗ��͂ɂȂ��Ă��܂�����l���̖��d�ɂ��v����");
		fontRenderer.AddString(glm::vec2(-430, -250), L"�_���W�����U�����n�܂����B");
		fontRenderer.AddString(glm::vec2(-200, -300), L"B�{�^���܂���K�{�^���Ń^�C�g���֖߂�܂�");

	}

	if (state == State::credit) {

		fontRenderer.AddString(glm::vec2(-430, 250), L"�^�C�g���@Title�@�@Defence�@of�@Soul");
		fontRenderer.AddString(glm::vec2(-430, 200), L"����ҁ@�@Creator�@�o���^���Q�[���A�J�f�~�[���Z");
		fontRenderer.AddString(glm::vec2(-430, 150), L"�@�@�@�@�@�@�@�@�@�h����@�đ��h�@Narikwa�@Shota");
		fontRenderer.AddString(glm::vec2(-430, 100), L"�d�l�f�ށ@Material�@used");
		fontRenderer.AddString(glm::vec2(-430, 50), L"�e�X�e�[�WBGM.SE�@�\�@�������@�@�@�@�@�@�@�l");
		fontRenderer.AddString(glm::vec2(-430, 10), L"�L�����N�^�[���@�@�\�@���ʉ����{�@�@�@�@�@�l");
		fontRenderer.AddString(glm::vec2(-430, -30), L"�A�C�R���摜�@�@�@�\�@ICOOON�@MONO�@�@�@�@�l");
		fontRenderer.AddString(glm::vec2(-430, -70), L"�e�L�X�g�摜�@�@�@�\�@COOL�@TEXT�@�@�@�@�@�l");
		fontRenderer.AddString(glm::vec2(-430, -110), L"�v���C���[�摜�@�@�\�@�p�u���b�N�h���C���@�l");
		fontRenderer.AddString(glm::vec2(-430, -150), L"�@�@�@�@�@�@�@�@�@�\�@���炷�Ƃ�@�@�@�@�@�l");
		fontRenderer.AddString(glm::vec2(-430, -190), L"�E�B���h�E�摜�@�@�\�@�т����[�f�ފف@�@�@�l");
		fontRenderer.AddString(glm::vec2(-430, -230), L"�Q�[���p�b�h�摜�@�\�@�G���Y�@�@�@�@�@�@�@�l");

		fontRenderer.AddString(glm::vec2(-200, -300), L"B�{�^���܂���K�{�^���Ń^�C�g���֖߂�܂�");

	}
	fontRenderer.EndUpdate();
}

/**
*�V�[����`�悷��.
*/

void TitleScene::Render() {

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());

	if (state == State::tutorial) {
		sprites[2].Scale(glm::vec2(1.08f, 0.11f));
		sprites[3].Scale(glm::vec2(0.1f));
		sprites[4].Scale(glm::vec2(0.1f));
		if (tutorialCount == 1) {
			//�I���A�C�R��.
			sprites[2].Position(glm::vec3(-260, -250, 0));
			sprites[3].Position(glm::vec3(-370, -250, 0));
			sprites[4].Position(glm::vec3(-150, -250, 0));
		}
		else if (tutorialCount == 2) {
			//�I���A�C�R��.
			sprites[2].Position(glm::vec3(0, -250, 0));
			sprites[3].Position(glm::vec3(-110, -250, 0));
			sprites[4].Position(glm::vec3(110, -250, 0));
		}
		else if (tutorialCount == 3) {
			//�I���A�C�R��.
			sprites[2].Position(glm::vec3(270, -250, 0));
			sprites[3].Position(glm::vec3(160, -250, 0));
			sprites[4].Position(glm::vec3(380, -250, 0));
		}
	}

	if (state == State::title) {
		sprites[2].Scale(glm::vec2(2.0f, 0.25f));
		sprites[3].Scale(glm::vec2(0.2f));
		sprites[4].Scale(glm::vec2(0.2f));
		if (selectCount == 1) {
			//�I���A�C�R��.
			sprites[2].Scale(glm::vec2(2.43f, 0.22f));
			sprites[2].Position(glm::vec3(0, -50, 0));
			sprites[3].Position(glm::vec3(-250, -50, 0));
			sprites[4].Position(glm::vec3(250, -50, 0));
		}
		else if (selectCount == 2) {
			//�I���A�C�R��.
			sprites[2].Scale(glm::vec2(1.46f, 0.22f));
			sprites[2].Position(glm::vec3(-5, -140, 0));
			sprites[3].Position(glm::vec3(-150, -140, 0));
			sprites[4].Position(glm::vec3(140, -140, 0));
		}
		else if (selectCount == 3) {
			//�I���A�C�R��.
			sprites[2].Scale(glm::vec2(1.6f, 0.22f));
			sprites[2].Position(glm::vec3(0, -230, 0));
			sprites[3].Position(glm::vec3(-160, -230, 0));
			sprites[4].Position(glm::vec3(160, -230, 0));
		}
		else if (selectCount == 4) {
			//�I���A�C�R��.
			sprites[2].Scale(glm::vec2(1.25f, 0.22f));
			sprites[2].Position(glm::vec3(0, -320, 0));
			sprites[3].Position(glm::vec3(-120, -320, 0));
			sprites[4].Position(glm::vec3(120, -320, 0));
		}
	}

	spriteRenderer.Draw(screenSize);
	fontRenderer.Draw(screenSize);
}
