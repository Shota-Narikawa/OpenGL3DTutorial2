/**
*@file Scene.cpp
*/

#include "GLFWEW.h"
#include "Scene.h"
#include<iostream>

/**
*�R���X�g���N�^.
*
*@param name	�V�[����.
*/

Scene::Scene(const char* name) : name(name) {

	std::cout << "Scene �R���X�g���N�^:" << name << "\n";
}

/**
*�f�X�g���N�^.
*/

Scene::~Scene() {

	Finalize();
	std::cout << "Scene �f�X�g���N�^:" << name << "\n";
}

/**
*�V�[����������Ԃɂ���.
*/

void Scene::Play() {

	isActive = true;
	std::cout << "Scene Play:" << name << "\n";
}

/**
*�V�[�����~��Ԃɂ���.
*/

void Scene::Stop() {

	isActive = false;
	std::cout << "Scene Stop:" << name << "\n";
}

/**
*�V�[����\������.
*/

void Scene::Show() {

	isVisible = true;
	std::cout << "Scene Show:" << name << "\n";
}

/**
*�V�[�����\���ɂ���.
*/

void Scene::Hide() {

	isVisible = false;
	std::cout << "Scene Hide:" << name << "\n";
}

/**
*�V�[�������擾����.
*
*@return	�V�[����.
*/

const std::string& Scene::Name()const {

	return name;
}

/**
*�V�[���̊�����Ԃ𒲂ׂ�.
*
*@retval true	�������Ă���.
*@retval false	��~���Ă���.
*/

bool Scene::IsActive()const {

	return isActive;
}

/**
*�V�[���̕\����Ԃ𒲂ׂ�.
*
*@retval true	�\�����.
*@retval false	��\�����.
*/

bool Scene::IsVisible()const {

	return isVisible;
}

/**
*�V�[���X�^�b�N���擾����.
*
*@return	�V�[���X�^�b�N.
*/

SceneStack& SceneStack::Instance() {

	static SceneStack instance;
	return instance;
}

/**
*�R���X�g���N�^.
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
*�V�[�����v�b�V������.
*
*@param p	�V�����V�[��.
*/

void SceneStack::Push(ScenePtr p) {

	if (!stack.empty()) {

		Current().Stop();
	}
	stack.push_back(p);
	std::cout << "[�V�[���v�b�V��]" << p->Name() << "\n";
	Current().Initialize();
	Current().Play();
}

/**
*�V�[�����|�b�v����.
*/

void SceneStack::Pop() {

	if (stack.empty()) {
		std::cout << "[�V�[�� �|�b�v][�x��]�V�[���X�^�b�N����ł�.\n";
		return;
	}
	Current().Stop();
	Current().Finalize();
	const std::string sceneName = Current().Name();
	stack.pop_back();
	std::cout << "[�V�[���@�|�b�v]" << sceneName << "\n";

	if (!stack.empty()) {
		Current().Play();
	}
}

/**
*�V�[����u��������.
*
*@param p	�V�����V�[��.
*/

void SceneStack::Replace(ScenePtr p) {

	std::string sceneName = "(Empty)";
	if (stack.empty()) {
		std::cout << "[�V�[���@���v���[�X][�x��]�V�[���X�^�b�N����ł�.\n";
	}
	else {
		sceneName = Current().Name();
		Current().Stop();
	}
	nextScene = p;
	fadeMode = FadeMode::out;
	std::cout << "[�V�[�� ���v���[�X]" << sceneName << "->" << p->Name() << "\n";
}

/**
*���݂̃V�[�����擾����.
*
*@return	���݂̃V�[��.
*/

Scene& SceneStack::Current() {

	return *stack.back();
}

/**
*���݂̃V�[�����擾����.
*
*@return	���݂̃V�[��.
*/

const Scene& SceneStack::Current() const {

	return *stack.back();
}

/**
*�V�[���̐����擾����.
*
*@return	�X�^�b�N�ɐς܂�Ă���V�[���̐�.
*/

size_t SceneStack::Size()const {

	return stack.size();
}


/**
*�X�^�b�N���󂩂ǂ������ׂ�.
*
*@retval true	�X�^�b�N�͋�.
*@retval false	�X�^�b�N�ɂP�ȏ�̃V�[�����ς܂�Ă���.
*/

bool SceneStack::Empty() const {

	return stack.empty();
}

/**
*�V�[�����X�V����.
*
*@param deltaTime	�O��̍X�V����̌o�ߎ��ԁi�b�j.
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
*�V�[����`�悷��.
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
* �t�F�[�h�C���E�t�F�[�h�A�E�g����N���X�̃V���O���g���C���X�^���X���擾����.
*
* @return �t�F�[�h�C���E�t�F�[�h�A�E�g����N���X�̃V���O���g���C���X�^���X.
*/
SceneFader& SceneFader::Instance()
{
	static SceneFader instance;
	return instance;
}

/**
* �R���X�g���N�^.
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
* �t�F�[�h�C���E�t�F�[�h�A�E�g�̏�Ԃ��X�V����.
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b).
*/
void SceneFader::Update(float deltaTime)
{
	if (mode != Mode::none) {
		// ��ʑS�̂𕢂��悤�ɃX�v���C�g�̃T�C�Y�𒲐�.
		glm::vec2 scale;
		GLFWEW::Window& window = GLFWEW::Window::Instance();
		scale.x = static_cast<float>(window.Width()) / static_cast<float>(spr.Texture()->Width());
		scale.y = static_cast<float>(window.Height()) / static_cast<float>(spr.Texture()->Height());
		spr.Scale(scale);

		// �o�ߎ��Ԃɉ����ĕs�����x���X�V.
		glm::vec4 color = spr.Color();
		if (mode == Mode::fadeIn) {
			color.a = timer / totalTime;
		}
		else if (mode == Mode::fadeOut) {
			color.a = 1.0f - timer / totalTime;
		}
		spr.Color(color);

		// �X�v���C�g���X�v���C�g�`��N���X�ɒǉ�.
		spriteRenderer.BeginUpdate();
		spriteRenderer.AddVertices(spr);
		spriteRenderer.EndUpdate();

		// �o�ߎ��ԃ^�C�}�[�X�V.
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
* �t�F�[�h�C���E�t�F�[�h�A�E�g��`�悷��.
*/
void SceneFader::Render() const
{
	if (mode != Mode::none) {
		GLFWEW::Window& window = GLFWEW::Window::Instance();
		spriteRenderer.Draw(glm::vec2(window.Width(), window.Height()));
	}
}

/**
* �t�F�[�h�A�E�g���J�n����.
*
* @param time  �t�F�[�h�A�E�g�ɂ����鎞��(�b).
* @param color �t�F�[�h�A�E�g��̐F.
*/
void SceneFader::FadeOut(float time, const glm::vec3& color)
{
	mode = Mode::fadeOut;
	totalTime = timer = time;
	spr.Color(glm::vec4(color, 1));
}

/**
* �t�F�[�h�C�����J�n����.
*
* @param time �t�F�[�h�C���ɂ����鎞��(�b).
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
* �t�F�[�h�C��(�܂��̓t�F�[�h�A�E�g)�������ׂ�.
*
* @retval true  �t�F�[�h�C��(�܂��̓t�F�[�h�A�E�g)��.
* @retval false �t�F�[�h�C��(�܂��̓t�F�[�h�A�E�g)�͏I�����Ă���.
*/
bool SceneFader::IsFading() const
{
	return mode != Mode::none && timer > 0;
}