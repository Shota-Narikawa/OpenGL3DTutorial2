/**
*@file StatusScene.h
*/

#ifndef STATUSSCENE_H_INCLUDED
#define STATUSSCENE_H_INCLUDED
#include "Scene.h"
#include "Font.h"

/**
*ステータス画面.
*/

class StatusScene : public Scene {

public:
	StatusScene() : Scene("StatusScene") {}
	virtual ~StatusScene() = default;

	virtual bool Initialize() override;
	virtual void ProcessInput() override;
	virtual void Update(float) override;
	virtual void Render() override;
	virtual void Finalize() override;

private:
	std::vector<Sprite> sprites;
	SpriteRenderer spriteRenderer;
	FontRenderer fontRenderer;

	float timer = 1.0f;
	bool isFinish = false;
	bool ButtonStart_f = true;
	bool ButtonEnd_f = false;

	float BlinkTimer;		//点滅間隔
	float btntimer = 0.1f;

	Texture::Image2D texCNumber[10];
	Shader::Program progSimple;
};

#endif // STATUSSCENE_H_INCLUDED

