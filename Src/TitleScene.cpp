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
*シーンを初期化する.
*
*@retval true	初期化成功.
*@retval false	初期化失敗.ゲーム進行不可につき、プログラムを終了すること.
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

	//選択アイコン.
	Sprite selectIcon3(Texture::Image2D::Create("Res/selectA.tga"));
	selectIcon3.Position(glm::vec3(-490, 210, 0));
	selectIcon3.Scale(glm::vec2(0));
	sprites.push_back(selectIcon3);

	//選択アイコン.
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

	//BGMを再生する.
	bgm = Audio::Engine::Instance().Prepare("Res/Audio/TitleScene2.mp3");
	bgm->Play(Audio::Flag_Loop);

	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/font.fnt");

	return true;
}

/**
*プレイヤーの入力を処理する.
*/

void TitleScene::ProcessInput() {

	GLFWEW::Window& window = GLFWEW::Window::Instance();

	if (state == State::title) {
		//選択アイコンを動かす.
		if (window.GetGamePad().buttonDown & GamePad::DPAD_UP) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			if (selectCount >= 1) {
				--selectCount;
				if (selectCount == 0) {
					selectCount = 4;
				}
			}
		}
		//おわる
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
		//選択アイコンを動かす.
		if (window.GetGamePad().buttonDown & GamePad::DPAD_LEFT) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			if (tutorialCount >= 1) {
				--tutorialCount;
				if (tutorialCount == 0) {
					tutorialCount = 3;
				}
			}
		}
		//おわる
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

	//操作確認画面時.
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

	//ストーリーかクレジット確認時.
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

	//タイトル画面時.
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
*シーンを更新する.
*
*@param deltaTime	前回の更新からの経過時間.
*/

void TitleScene::Update(float deltaTime) {

	spriteRenderer.BeginUpdate();

	for (const Sprite& e : sprites) {

		spriteRenderer.AddVertices(e);
	}
	spriteRenderer.EndUpdate();

	//ロード画面中.
	if (loadTimer > 0.0f) {
		loadTimer -= deltaTime;
		if (loadTimer <= 0.0f) {
			SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
			return;
		}
	}

	//シーン切り替え待ち.
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
		fontRenderer.AddString(glm::vec2(-280, 260), L"ゲームを初める前に操作の確認をしよう");
		fontRenderer.AddString(glm::vec2(-355, -260), L"ゲームパッド");
		fontRenderer.AddString(glm::vec2(-80, -260), L"キーボード");
		fontRenderer.AddString(glm::vec2(175, -260), L"本編を始める");

		if (tutorialText) {
			fontRenderer.Color(glm::vec4(0.0f, 0.0f, 0.0f, 1));
			fontRenderer.AddString(glm::vec2(-340, 140), L"視点左回転");
			fontRenderer.AddString(glm::vec2(-310, 10), L"決定");
			fontRenderer.AddString(glm::vec2(-340, -50), L"メニュー");
			fontRenderer.AddString(glm::vec2(-310, -120), L"移動");

			fontRenderer.AddString(glm::vec2(215, 200), L"ジャンプ");
			fontRenderer.AddString(glm::vec2(215, 140), L"視点右回転");
			fontRenderer.AddString(glm::vec2(215, 75), L"スキル１");
			fontRenderer.AddString(glm::vec2(215, 10), L"スキル２");
			fontRenderer.AddString(glm::vec2(215, -55), L"スキル３");
			fontRenderer.AddString(glm::vec2(215, -115), L"スキル４");
		}
		else
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-460, 210), L"WSAD:移動");
			fontRenderer.AddString(glm::vec2(-460, 170), L"L：ジャンプ");
			fontRenderer.AddString(glm::vec2(-460, 130), L"U：スキル１");
			fontRenderer.AddString(glm::vec2(-460, 90), L"H：スキル２");
			fontRenderer.AddString(glm::vec2(-460, 50), L"K：スキル３");
			fontRenderer.AddString(glm::vec2(-460, 10), L"J：スキル４");
			fontRenderer.AddString(glm::vec2(-460, -30), L"Q：カメラ左回転");
			fontRenderer.AddString(glm::vec2(-460, -70), L"E：カメラ右回転");
			fontRenderer.AddString(glm::vec2(-460, -110), L"SPACE:メニュー");
			fontRenderer.AddString(glm::vec2(-460, -150), L"ENTER:決定");
		}
	}

	if (state == State::load) {
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 10));
		fontRenderer.AddString(glm::vec2(-100, 0), L"NowLoading...");
	}

	if (state == State::story) {
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
		fontRenderer.AddString(glm::vec2(-430, 250), L"これはとある世界の、とある１人の冒険者の物語である。");
		fontRenderer.AddString(glm::vec2(-430, 200), L"ある日、冒険者は高難易度のダンジョンへ冒険に出ていた。");
		fontRenderer.AddString(glm::vec2(-430, 150), L"しかし、ダンジョンで敵に敗北し、囚われの身となってしまう。");
		fontRenderer.AddString(glm::vec2(-430, 100), L"敵は人間の魂のみを喰らう”ソウルイーター”だった。");
		fontRenderer.AddString(glm::vec2(-430, 50), L"肉体と魂を分離されたが、喰われる直前に脱走し地上へ向かった。");
		fontRenderer.AddString(glm::vec2(-430, 0), L"だが、地上付近には敵が大勢いて抜け出そうにもない。");
		fontRenderer.AddString(glm::vec2(-430, -50), L"逃げ切れたとしてもこの状態では地上では生きていけない。");
		fontRenderer.AddString(glm::vec2(-430, -100), L"もう敵に奪われている自分の体を取り戻すしかない。");
		fontRenderer.AddString(glm::vec2(-430, -150), L"彼は再びダンジョンに潜り、取り戻す決意をした。");
		fontRenderer.AddString(glm::vec2(-430, -200), L"そんな無力になってしまった主人公の無謀にも思える");
		fontRenderer.AddString(glm::vec2(-430, -250), L"ダンジョン攻略が始まった。");
		fontRenderer.AddString(glm::vec2(-200, -300), L"BボタンまたはKボタンでタイトルへ戻ります");

	}

	if (state == State::credit) {

		fontRenderer.AddString(glm::vec2(-430, 250), L"タイトル　Title　　Defence　of　Soul");
		fontRenderer.AddString(glm::vec2(-430, 200), L"制作者　　Creator　バンタンゲームアカデミー大阪校");
		fontRenderer.AddString(glm::vec2(-430, 150), L"　　　　　　　　　”成川　翔太”　Narikwa　Shota");
		fontRenderer.AddString(glm::vec2(-430, 100), L"仕様素材　Material　used");
		fontRenderer.AddString(glm::vec2(-430, 50), L"各ステージBGM.SE　―　魔王魂　　　　　　　様");
		fontRenderer.AddString(glm::vec2(-430, 10), L"キャラクター声　　―　効果音ラボ　　　　　様");
		fontRenderer.AddString(glm::vec2(-430, -30), L"アイコン画像　　　―　ICOOON　MONO　　　　様");
		fontRenderer.AddString(glm::vec2(-430, -70), L"テキスト画像　　　―　COOL　TEXT　　　　　様");
		fontRenderer.AddString(glm::vec2(-430, -110), L"プレイヤー画像　　―　パブリックドメイン　様");
		fontRenderer.AddString(glm::vec2(-430, -150), L"　　　　　　　　　―　いらすとや　　　　　様");
		fontRenderer.AddString(glm::vec2(-430, -190), L"ウィンドウ画像　　―　びたちー素材館　　　様");
		fontRenderer.AddString(glm::vec2(-430, -230), L"ゲームパッド画像　―　絵太郎　　　　　　　様");

		fontRenderer.AddString(glm::vec2(-200, -300), L"BボタンまたはKボタンでタイトルへ戻ります");

	}
	fontRenderer.EndUpdate();
}

/**
*シーンを描画する.
*/

void TitleScene::Render() {

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());

	if (state == State::tutorial) {
		sprites[2].Scale(glm::vec2(1.08f, 0.11f));
		sprites[3].Scale(glm::vec2(0.1f));
		sprites[4].Scale(glm::vec2(0.1f));
		if (tutorialCount == 1) {
			//選択アイコン.
			sprites[2].Position(glm::vec3(-260, -250, 0));
			sprites[3].Position(glm::vec3(-370, -250, 0));
			sprites[4].Position(glm::vec3(-150, -250, 0));
		}
		else if (tutorialCount == 2) {
			//選択アイコン.
			sprites[2].Position(glm::vec3(0, -250, 0));
			sprites[3].Position(glm::vec3(-110, -250, 0));
			sprites[4].Position(glm::vec3(110, -250, 0));
		}
		else if (tutorialCount == 3) {
			//選択アイコン.
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
			//選択アイコン.
			sprites[2].Scale(glm::vec2(2.43f, 0.22f));
			sprites[2].Position(glm::vec3(0, -50, 0));
			sprites[3].Position(glm::vec3(-250, -50, 0));
			sprites[4].Position(glm::vec3(250, -50, 0));
		}
		else if (selectCount == 2) {
			//選択アイコン.
			sprites[2].Scale(glm::vec2(1.46f, 0.22f));
			sprites[2].Position(glm::vec3(-5, -140, 0));
			sprites[3].Position(glm::vec3(-150, -140, 0));
			sprites[4].Position(glm::vec3(140, -140, 0));
		}
		else if (selectCount == 3) {
			//選択アイコン.
			sprites[2].Scale(glm::vec2(1.6f, 0.22f));
			sprites[2].Position(glm::vec3(0, -230, 0));
			sprites[3].Position(glm::vec3(-160, -230, 0));
			sprites[4].Position(glm::vec3(160, -230, 0));
		}
		else if (selectCount == 4) {
			//選択アイコン.
			sprites[2].Scale(glm::vec2(1.25f, 0.22f));
			sprites[2].Position(glm::vec3(0, -320, 0));
			sprites[3].Position(glm::vec3(-120, -320, 0));
			sprites[4].Position(glm::vec3(120, -320, 0));
		}
	}

	spriteRenderer.Draw(screenSize);
	fontRenderer.Draw(screenSize);
}
