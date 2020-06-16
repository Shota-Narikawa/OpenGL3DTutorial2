/**
*@file Scene.cpp
*/

#include "GLFWEW.h"
#include "Scene.h"
#include<iostream>

/**
*コンストラクタ.
*
*@param name	シーン名.
*/

Scene::Scene(const char* name) : name(name) {

	std::cout << "Scene コンストラクタ:" << name << "\n";
}

/**
*デストラクタ.
*/

Scene::~Scene() {

	Finalize();
	std::cout << "Scene デストラクタ:" << name << "\n";
}

/**
*シーンを活動状態にする.
*/

void Scene::Play() {

	isActive = true;
	std::cout << "Scene Play:" << name << "\n";
}

/**
*シーンを停止状態にする.
*/

void Scene::Stop() {

	isActive = false;
	std::cout << "Scene Stop:" << name << "\n";
}

/**
*シーンを表示する.
*/

void Scene::Show() {

	isVisible = true;
	std::cout << "Scene Show:" << name << "\n";
}

/**
*シーンを非表示にする.
*/

void Scene::Hide() {

	isVisible = false;
	std::cout << "Scene Hide:" << name << "\n";
}

/**
*シーン名を取得する.
*
*@return	シーン名.
*/

const std::string& Scene::Name()const {

	return name;
}

/**
*シーンの活動状態を調べる.
*
*@retval true	活動している.
*@retval false	停止している.
*/

bool Scene::IsActive()const {

	return isActive;
}

/**
*シーンの表示状態を調べる.
*
*@retval true	表示状態.
*@retval false	非表示状態.
*/

bool Scene::IsVisible()const {

	return isVisible;
}

/**
*シーンスタックを取得する.
*
*@return	シーンスタック.
*/

SceneStack& SceneStack::Instance() {

	static SceneStack instance;
	return instance;
}

/**
*コンストラクタ.
*/

SceneStack::SceneStack() {

	stack.reserve(16);
	spriteRenderer.Init(10, "Res/Sprite.vert", "Res/Sprite.frag");
	sprFader = Sprite(Texture::Image2D::Create("Res/white4x4.tga"));
	sprFader.Color(glm::vec4(0, 0, 0, 1));
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	sprFader.Scale(glm::vec2(static_cast<float>(window.Width()) / 4.0f, static_cast<float>(window.Height()) / 4.0f));
}

/**
*シーンをプッシュする.
*
*@param p	新しいシーン.
*/

void SceneStack::Push(ScenePtr p) {

	if (!stack.empty()) {

		Current().Stop();
	}
	stack.push_back(p);
	std::cout << "[シーンプッシュ]" << p->Name() << "\n";
	Current().Initialize();
	Current().Play();
}

/**
*シーンをポップする.
*/

void SceneStack::Pop() {

	if (stack.empty()) {
		std::cout << "[シーン ポップ][警告]シーンスタックが空です.\n";
		return;
	}
	Current().Stop();
	Current().Finalize();
	const std::string sceneName = Current().Name();
	stack.pop_back();
	std::cout << "[シーン　ポップ]" << sceneName << "\n";

	if (!stack.empty()) {
		Current().Play();
	}
}

/**
*シーンを置き換える.
*
*@param p	新しいシーン.
*/

void SceneStack::Replace(ScenePtr p) {

	std::string sceneName = "(Empty)";
	if (stack.empty()) {
		std::cout << "[シーン　リプレース][警告]シーンスタックが空です.\n";
	}
	else {
		sceneName = Current().Name();
		Current().Stop();
	}
	nextScene = p;
	fadeMode = FadeMode::out;
	std::cout << "[シーン リプレース]" << sceneName << "->" << p->Name() << "\n";
}

/**
*現在のシーンを取得する.
*
*@return	現在のシーン.
*/

Scene& SceneStack::Current() {

	return *stack.back();
}

/**
*現在のシーンを取得する.
*
*@return	現在のシーン.
*/

const Scene& SceneStack::Current() const {

	return *stack.back();
}

/**
*シーンの数を取得する.
*
*@return	スタックに積まれているシーンの数.
*/

size_t SceneStack::Size()const {

	return stack.size();
}


/**
*スタックが空かどうか調べる.
*
*@retval true	スタックは空.
*@retval false	スタックに１つ以上のシーンが積まれている.
*/

bool SceneStack::Empty() const {

	return stack.empty();
}

/**
*シーンを更新する.
*
*@param deltaTime	前回の更新からの経過時間（秒）.
*/

void SceneStack::Update(float deltaTime) {


	if (!Empty()) {

		Current().ProcessInput();
	}

	for (ScenePtr& e : stack) {
		if (e->IsActive()) {
			e->Update(deltaTime);
		}
	}
	const float fadeSpeed = 2;
	glm::vec4 c = sprFader.Color();
	if (fadeMode == FadeMode::in) {
		c.a -= fadeSpeed * deltaTime;
		if (c.a <= 0) {
			c.a = 0;
			fadeMode = FadeMode::none;
		}
	}
	else if (fadeMode == FadeMode::out) {
		c.a += fadeSpeed * deltaTime;
		if (c.a >= 1) {
			c.a = 1;
			if (nextScene) {
				Current().Finalize();
				stack.pop_back();
				stack.push_back(nextScene);
				Current().Initialize();
				Current().Play();
				fadeMode = FadeMode::in;
			}
			else {
				fadeMode = FadeMode::none;
			}
		}
	}
	sprFader.Color(c);
	spriteRenderer.BeginUpdate();
	spriteRenderer.AddVertices(sprFader);
	spriteRenderer.EndUpdate();
}

/**
*シーンを描画する.
*/

void SceneStack::Render() {

	for (ScenePtr& e : stack) {
		if (e->IsVisible()) {
			e->Render();
		}
	}
	if (sprFader.Color().a > 0) {
		const GLFWEW::Window& window = GLFWEW::Window::Instance();
		spriteRenderer.Draw(glm::vec2(window.Width(), window.Height()));
	}
}

/**
* フェードイン・フェードアウト制御クラスのシングルトンインスタンスを取得する.
*
* @return フェードイン・フェードアウト制御クラスのシングルトンインスタンス.
*/
SceneFader& SceneFader::Instance()
{
	static SceneFader instance;
	return instance;
}

/**
* コンストラクタ.
*/
SceneFader::SceneFader()
{
	uint32_t imageData[4 * 4];
	for (auto& e : imageData) {
		e = 0xffffffff;
	}
	spriteRenderer.Init(1, "Res/Sprite.vert", "Res/Sprite.frag");
	GLuint texId = Texture::CreateImage2D(4, 4, imageData, GL_BGRA, GL_UNSIGNED_BYTE);
	spr = Sprite(std::make_shared<Texture::Image2D>(texId));
}

/**
* フェードイン・フェードアウトの状態を更新する.
*
* @param deltaTime 前回の更新からの経過時間(秒).
*/
void SceneFader::Update(float deltaTime)
{
	if (mode != Mode::none) {
		// 画面全体を覆うようにスプライトのサイズを調整.
		glm::vec2 scale;
		GLFWEW::Window& window = GLFWEW::Window::Instance();
		scale.x = static_cast<float>(window.Width()) / static_cast<float>(spr.Texture()->Width());
		scale.y = static_cast<float>(window.Height()) / static_cast<float>(spr.Texture()->Height());
		spr.Scale(scale);

		// 経過時間に応じて不透明度を更新.
		glm::vec4 color = spr.Color();
		if (mode == Mode::fadeIn) {
			color.a = timer / totalTime;
		}
		else if (mode == Mode::fadeOut) {
			color.a = 1.0f - timer / totalTime;
		}
		spr.Color(color);

		// スプライトをスプライト描画クラスに追加.
		spriteRenderer.BeginUpdate();
		spriteRenderer.AddVertices(spr);
		spriteRenderer.EndUpdate();

		// 経過時間タイマー更新.
		timer -= deltaTime;
		if (timer <= 0) {
			timer = 0;
			if (mode == Mode::fadeIn) {
				mode = Mode::none;
			}
		}
	}
}

/**
* フェードイン・フェードアウトを描画する.
*/
void SceneFader::Render() const
{
	if (mode != Mode::none) {
		GLFWEW::Window& window = GLFWEW::Window::Instance();
		spriteRenderer.Draw(glm::vec2(window.Width(), window.Height()));
	}
}

/**
* フェードアウトを開始する.
*
* @param time  フェードアウトにかかる時間(秒).
* @param color フェードアウト先の色.
*/
void SceneFader::FadeOut(float time, const glm::vec3& color)
{
	mode = Mode::fadeOut;
	totalTime = timer = time;
	spr.Color(glm::vec4(color, 1));
}

/**
* フェードインを開始する.
*
* @param time フェードインにかかる時間(秒).
*/
void SceneFader::FadeIn(float time)
{
	mode = Mode::fadeIn;
	totalTime = timer = time;
	glm::vec4 c = spr.Color();
	c.a = 0;
	spr.Color(c);
}

/**
* フェードイン(またはフェードアウト)中か調べる.
*
* @retval true  フェードイン(またはフェードアウト)中.
* @retval false フェードイン(またはフェードアウト)は終了している.
*/
bool SceneFader::IsFading() const
{
	return mode != Mode::none && timer > 0;
}