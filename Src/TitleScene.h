/**
*@file TitleScene.h
*/

#ifndef TITLESCENE_H_INCLUDED
#define TITLESCENE_H_INCLUDED
#include "Scene.h"
#include "Sprite.h"
#include "Font.h"
#include "Audio/Audio.h"
#include <vector>

/**
*タイトル画面.
*/

class TitleScene : public Scene {

public:
	TitleScene() : Scene("TitleScene") {}
	virtual ~TitleScene() = default;

	virtual bool Initialize() override;
	virtual void ProcessInput() override;
	virtual void Update(float) override;
	virtual void Render() override;
	virtual void Finalize() override {}

private:
	std::vector<Sprite> sprites;
	SpriteRenderer spriteRenderer;
	FontRenderer fontRenderer;

	//ゲーム状態.
	enum class State {
		title,		//タイトル.
		story, 		//ゲームのあらすじ.
		credit,		//クレジット情報.
		tutorial,	//ゲームパッド、キーボードの操作説明画面.
		load,		//ロード画面.
	};
	State state = State::title;

	float loadTimer = 0.0f;
	float timer = 0;
	Audio::SoundPtr bgm;
	bool isNext = false;
	bool tutorialText = false;

	int selectCount = 1;
	int tutorialCount = 0;
};

#endif // TITLESCENE_H_INCLUDED
