/**
*@file Sprite.h
*/

#ifndef SPRITE_H_INCLUDED
#define SPRITE_H_INCLUDED
#include "BufferObject.h"
#include "Texture.h"
#include "Shader.h"
#include "Actor.h"
#include <glm\glm.hpp>
#include <vector>


/**
*	�Z�`�\����.
*/

struct Rect
{
	glm::vec2 origin;	///<�������_.
	glm::vec2 size;		///<�c���̕�.
};

/**
*�X�v���C�g�N���X.
*/

class Sprite {

public:
	Sprite() = default;
	explicit Sprite(const Texture::Image2DPtr&);
	~Sprite() = default;
	Sprite(const Sprite&) = default;
	Sprite& operator=(const Sprite&) = default;

	//���W�̐ݒ�E�擾.
	void Position(const glm::vec3& p) { position = p; }
	const glm::vec3& Position() const { return position; }

	//��]�̐ݒ�E�擾.
	void Rotation(float r) { rotation = r; }
	float Rotation() const { return rotation; }

	//�g�嗦�̐ݒ�E�擾.
	void Scale(const glm::vec2& s) { scale = s; }
	const glm::vec2& Scale() const { return scale; }

	//�F�̐ݒ�E�擾.
	void Color(const glm::vec4& c) { color = c; }
	const glm::vec4& Color() const { return color; }

	//�Z�`�̐ݒ�E�擾.
	void Rectangle(const Rect& r) { rect = r; }
	const Rect& Rectangle() const { return rect; }

	//�e�N�X�`���̐ݒ�E�擾.
	void Texture(const Texture::Image2DPtr& tex);
	const Texture::Image2DPtr& Texture() const { return texture; }

	int id = -1;
	//10	���j���[���.
	//9		�Q�[���N���A�A�Q�[���I�[�o�[.
	//8		�I���A�C�R��.
	//7		���U���g���.
	//6		�X�L�����.
	//5		�v���C���[�����.
	//4		�v���C���.
	//3		���[�h���.

private:

	glm::vec3 position = glm::vec3(0);
	glm::f32 rotation = 0;
	glm::vec2 scale = glm::vec2(1);
	glm::vec4 color = glm::vec4(1);
	Rect rect = { glm::vec2(0,0),glm::vec2(1,1) };
	Texture::Image2DPtr texture;
};

/**
*�X�v���C�g�`��N���X.
*/

class SpriteRenderer {

public:
	SpriteRenderer() = default;
	~SpriteRenderer() = default;
	SpriteRenderer(const SpriteRenderer&) = delete;
	SpriteRenderer& operator=(const SpriteRenderer&) = delete;

	bool Init(size_t maxSpriteCount, const char* vsPath, const char* fsPath);
	void BeginUpdate();
	bool AddVertices(const Sprite&);
	void EndUpdate();
	void Draw(const glm::vec2&) const;
	void Clear();

	void DeleteSprite(std::vector<Sprite>& sprites, int i[]);
	void GameSceneUI(std::vector<Sprite>& sprites);
	void SpriteChange(std::vector<Sprite>& sprites, int selCou, int skCou, int id);
	void SprRootChange(std::vector<Sprite>& sprites, int other, int id);
	void MiniMap(std::vector<Sprite>& sprites, ActorList& a,bool flag, glm::vec3 pos);
	void DefenceUI(float DefLine, std::vector<Sprite>& sprites);
	void comboUI(float combo, float comTimer, float comBuf, std::vector<Sprite>& sprites);
	void pCommandUI(int pID, int pAb, std::vector<Sprite>& sprites);
	void comIntUI(std::vector<Sprite>& sprites, int pAb,int pID, bool s, bool e, bool n, bool w,
					float eT, float nT, float wT, float deltaTime);

private:
	BufferObject vbo;
	BufferObject ibo;
	VertexArrayObject vao;
	Shader::ProgramPtr program;

	struct Vertex
	{
		glm::vec3 position;	///<���W.
		glm::vec4 color;	///<�F.
		glm::vec2 texCoord;	///<�e�N�X�`�����W.
	};

	std::vector<Vertex> vertices;	//���_�f�[�^�z��.

	struct Primitive
	{
		size_t count;
		size_t offset;
		Texture::Image2DPtr texture;
	};
	std::vector<Primitive> primitives;

	struct MiniMapIcon
	{
		glm::vec3 position = glm::vec3(530, 280, 0);
		glm::vec3 velocity = glm::vec3(0);

		float width = 1280 / 5; ///< ��ʂ̕�(�s�N�Z����).
		float height = 720 / 5; ///< ��ʂ̍���(�s�N�Z����).
	};
	MiniMapIcon mapIcon;
};

void DeleteSpriteA(std::vector<Sprite>& sprites, int id);
void DeleteSpriteB(std::vector<Sprite>& sprites, int id);

#endif // !SPRITE_H_INCLUDED
