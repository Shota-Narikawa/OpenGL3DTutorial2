/**
*@file Sprite.cpp
*/

#include "Sprite.h"
#include <vector>
#include <iostream>
#include <glm\gtc\matrix_transform.hpp>

/**
*Sprite�R���X�g���N�^.
*
*@param tex	�X�v���C�g�Ƃ��ĕ\������e�N�X�`��.
*/
Sprite::Sprite(const Texture::Image2DPtr& tex) :
	texture(tex),
	rect(Rect{ glm::vec2(),glm::vec2(tex->Width(),tex->Height()) }) {

}

/**
*�`��Ɏg�p����e�N�X�`�����w�肷��.
*
*@param tex	�`��Ɏg�p����e�N�X�`��.
*/

void Sprite::Texture(const Texture::Image2DPtr& tex) {

	texture = tex;
	Rectangle(Rect{ glm::vec2(0),glm::vec2(tex->Width(),tex->Height()) });
}

/**
*�X�v���C�g�`��N���X������������.
*
*@param maxSpriteCount	�`��\�ȍő�X�v���C�g��.
*@param vsPath			���_�V�F�[�_�[�t�@�C����.
*@param fsPath			�t���O�����g�V�F�[�_�[�t�@�C����.
*
*@retval true	����������.
*@retval false	���������s.
*/

bool SpriteRenderer::Init(size_t maxSpriteCount, const char* vsPath, const char* fsPath) {

	vbo.Create(GL_ARRAY_BUFFER, sizeof(Vertex) * maxSpriteCount * 4,
		nullptr, GL_STREAM_DRAW);

	//�l�p�`��maxSpriteCount���.
	std::vector<GLushort> indices;
	indices.resize(maxSpriteCount * 6);	//�l�p�`���ƂɃC���f�b�N�X��6�K�v.
	for (GLushort i = 0; i < maxSpriteCount; ++i) {

		indices[i * 6 + 0] = (i * 4) + 0;
		indices[i * 6 + 1] = (i * 4) + 1;
		indices[i * 6 + 2] = (i * 4) + 2;
		indices[i * 6 + 3] = (i * 4) + 2;
		indices[i * 6 + 4] = (i * 4) + 3;
		indices[i * 6 + 5] = (i * 4) + 0;

	}

	ibo.Create(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort),
		indices.data(), GL_STATIC_DRAW);

	//Vertex�\���̂ɍ��킹�Ē��_�A�g���r���[�g��ݒ�.
	vao.Create(vbo.Id(), ibo.Id());
	vao.Bind();
	vao.VertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, position));
	vao.VertexAttribPointer(
		1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, color));
	vao.VertexAttribPointer(
		2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, texCoord));
	vao.Unbind();

	program = Shader::Program::Create(vsPath, fsPath);

	primitives.reserve(64);	//32�ł͑���Ȃ����Ƃ����邩������Ȃ��̂�64�\��.

	//�������ߒ��̂ǂꂩ�̃I�u�W�F�N�g�̍쐬�Ɏ��s���Ă�����A���̊֐����̂����s�Ƃ���.
	if (!vbo.Id() || !ibo.Id() || !vao.Id() || program->IsNull()) {

		return false;
	}

	return true;
}

/**
*���_�f�[�^�̍쐬���J�n����.
*/
void SpriteRenderer::BeginUpdate() {

	primitives.clear();
	vertices.clear();
	vertices.reserve(vbo.Size() / sizeof(Vertex));
}

/**
*���_�f�[�^��ǉ�����.
*
*@param sprite	���_�f�[�^�̌��ɂȂ�X�v���C�g.
*
*@retval true	�ǉ�����.
*@retval false	���_�o�b�t�@�����t�Œǉ��ł��Ȃ�.
*/

bool SpriteRenderer::AddVertices(const Sprite& sprite) {

	if (vertices.size() * sizeof(Vertex) >= static_cast<size_t>(vbo.Size())) {

		std::cerr << "[�x��]" << __func__ << ":�ő�\�����𒴂��Ă��܂�.\n";
		return false;
	}

	const Texture::Image2DPtr& texture = sprite.Texture();
	const glm::vec2 reciprocalSize(
		glm::vec2(1) / glm::vec2(texture->Width(), texture->Height()));

	//�Z�`��0.0�`1.0�͈̔͂ɕϊ�.
	Rect rect = sprite.Rectangle();
	rect.origin *= reciprocalSize;
	rect.size *= reciprocalSize;

	//���S����̑傫�����v�Z.
	const glm::vec2 halfSize = sprite.Rectangle().size * 0.5f;

	//���W�ϊ��s����쐬.
	const glm::mat4 matT = glm::translate(glm::mat4(1), sprite.Position());
	const glm::mat4 matR = glm::rotate(glm::mat4(1), sprite.Rotation(), glm::vec3(0, 0, 1));
	const glm::mat4 matS = glm::scale(glm::mat4(1), glm::vec3(sprite.Scale(), 1));
	const glm::mat4 transform = matT * matR * matS;

	Vertex v[4];

	v[0].position = transform * glm::vec4(-halfSize.x, -halfSize.y, 0, 1);
	v[0].color = sprite.Color();
	v[0].texCoord = rect.origin;

	v[1].position = transform * glm::vec4(halfSize.x, -halfSize.y, 0, 1);
	v[1].color = sprite.Color();
	v[1].texCoord = glm::vec2(rect.origin.x + rect.size.x, rect.origin.y);

	v[2].position = transform * glm::vec4(halfSize.x, halfSize.y, 0, 1);
	v[2].color = sprite.Color();
	v[2].texCoord = rect.origin + rect.size;

	v[3].position = transform * glm::vec4(-halfSize.x, halfSize.y, 0, 1);
	v[3].color = sprite.Color();
	v[3].texCoord = glm::vec2(rect.origin.x, rect.origin.y + rect.size.y);

	vertices.insert(vertices.end(), v, v + 4);

	if (primitives.empty()) {

		//�ŏ��̃v���~�e�B�u���쐬����.
		primitives.push_back({ 6,0,texture });

	}
	else {
		//�����e�N�X�`�����g���Ă���Ȃ�C���f�b�N�X���Ǝl�p�`�ЂƂԂ�i�C���f�b�N�X6�j���₷.
		//�e�N�X�`�����g���ꍇ�͐V�����v���~�e�B�u���쐬����.
		Primitive& data = primitives.back();

		if (data.texture == texture) {
			data.count += 6;
		}
		else {
			primitives.push_back({ 6,data.offset + data.count * sizeof(GLushort),texture });
		}
	}

	return true;
}

/**
*���_�f�[�^�̍쐬���I������.
*/

void SpriteRenderer::EndUpdate() {

	vbo.BufferSubData(0, vertices.size() * sizeof(Vertex), vertices.data());
	vertices.clear();
	vertices.shrink_to_fit();
}

/**
*�X�v���C�g��`�悷��.
*
*@param texture		�`��Ɏg�p����e�N�X�`��.
*@paraam screenSize	��ʃT�C�Y.
*/

void SpriteRenderer::Draw(const glm::vec2& screenSize)const {

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	vao.Bind();
	program->Use();

	//���s���e�A���_�͉�ʂ̒��S.
	const glm::vec2 halfScreenSize = screenSize * 0.5f;
	const glm::mat4x4 matProj = glm::ortho(
		-halfScreenSize.x, halfScreenSize.x, -halfScreenSize.y, halfScreenSize.y, 1.0f, 1000.0f);
	const glm::mat4x4 matView = glm::lookAt(
		glm::vec3(0, 0, 100), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	program->SetViewProjectionMatrix(matProj * matView);

	for (const Primitive& primitive : primitives) {
		program->BindTexture(0, primitive.texture->Get());
		glDrawElements(GL_TRIANGLES, primitive.count, GL_UNSIGNED_SHORT,
			reinterpret_cast<const GLvoid*>(primitive.offset));
	}
	program->BindTexture(0, 0);
	vao.Unbind();
}

/*
*�����X�v���C�gID���폜����.
*/
void DeleteSpriteA(std::vector<Sprite>& sprites, int id) {

	for (auto i = sprites.begin(); i != sprites.end();) {
		if (i->id == id) {
			i = sprites.erase(i);
		}
		else {
			++i;
		}
	}
}

/*
*�Y������X�v���C�gID��S�č폜����.
*/
void DeleteSprite(std::vector<Sprite>& sprites, int i[]) {

	sprites[*i].Scale(glm::vec2(0));
}

/*
*�Q�[�����Ŏg�p����摜�f�[�^���܂Ƃ߂ĊǗ�.
*
*@param	sprites	�\������X�v���C�g.
*/
void SpriteRenderer::GameSceneUI(std::vector<Sprite>& sprites) {

	//�Q�[�����ɕ\��������摜���X�g.
	//�ϐ��̍Ō��No�����Ă킩��₷�����Ă���.
	//���[�h���.
	Sprite road0(Texture::Image2D::Create("Res/Black.tga"));
	road0.Scale(glm::vec2(0));
	sprites.push_back(road0);

	//���C�����(�E).
	Sprite Menu1(Texture::Image2D::Create("Res/MenuWindow5.tga"));
	Menu1.Position(glm::vec3(160, 0, 0));
	Menu1.Scale(glm::vec2(0));
	sprites.push_back(Menu1);

	//���j���[�ꗗ(����).
	Sprite Menu2(Texture::Image2D::Create("Res/MenuWindow5.tga"));
	Menu2.Position(glm::vec3(-380, 85, 0));
	Menu2.Scale(glm::vec2(0));
	sprites.push_back(Menu2);

	//���̑�(����).
	Sprite Menu3(Texture::Image2D::Create("Res/MenuWindow5.tga"));
	Menu3.Position(glm::vec3(-380, -220, 0));
	Menu3.Scale(glm::vec2(0));
	sprites.push_back(Menu3);

	//���[�h��ʒ��̉摜.
	Sprite LoadA4(Texture::Image2D::Create("Res/LoadA.tga"));
	LoadA4.Position(glm::vec3(0, 130, 0));
	LoadA4.Scale(glm::vec2(0));
	sprites.push_back(LoadA4);

	Sprite LoadB5(Texture::Image2D::Create("Res/LoadB.tga"));
	LoadB5.Position(glm::vec3(0, 130, 0));
	LoadB5.Scale(glm::vec2(0));
	sprites.push_back(LoadB5);

	Sprite LoadC6(Texture::Image2D::Create("Res/LoadC.tga"));
	LoadC6.Position(glm::vec3(0, 130, 0));
	LoadC6.Scale(glm::vec2(0));
	sprites.push_back(LoadC6);

	Sprite LoadD7(Texture::Image2D::Create("Res/LoadD.tga"));
	LoadD7.Position(glm::vec3(0, 100, 0));
	LoadD7.Scale(glm::vec2(0));
	sprites.push_back(LoadD7);

	//���U���g��ʂ̃��S.
	Sprite NextLogo8(Texture::Image2D::Create("Res/Next.tga"));
	NextLogo8.Position(glm::vec3(100, 120, 0));
	NextLogo8.Scale(glm::vec2(0));
	sprites.push_back(NextLogo8);

	Sprite ToTitle9(Texture::Image2D::Create("Res/ToTitle.tga"));
	ToTitle9.Position(glm::vec3(100, -120, 0));
	ToTitle9.Scale(glm::vec2(0));
	sprites.push_back(ToTitle9);

	//�Q�[���I�[�o�[��.
	Sprite GameOver10(Texture::Image2D::Create("Res/Over.tga"));
	GameOver10.Scale(glm::vec2(0));
	GameOver10.Position(glm::vec3(0, 100, 0));
	sprites.push_back(GameOver10);

	//�Q�[���N���A��.
	Sprite GameClear11(Texture::Image2D::Create("Res/Clear.tga"));
	GameClear11.Scale(glm::vec2(0));
	GameClear11.Position(glm::vec3(0, 100, 0));
	sprites.push_back(GameClear11);

	//���j���[��ʂ̃v���C���[�A�C�R��.
	Sprite pSoul12(Texture::Image2D::Create("Res/pSoul.tga"));
	pSoul12.Position(glm::vec3(-50, 110, 0));
	pSoul12.Scale(glm::vec2(0));
	sprites.push_back(pSoul12);

	Sprite pGoblin13(Texture::Image2D::Create("Res/pGoblin.tga"));
	pGoblin13.Position(glm::vec3(-50, 110, 0));
	pGoblin13.Scale(glm::vec2(0));
	sprites.push_back(pGoblin13);

	Sprite pWizard14(Texture::Image2D::Create("Res/pWizard.tga"));
	pWizard14.Position(glm::vec3(-50, 110, 0));
	pWizard14.Scale(glm::vec2(0));
	sprites.push_back(pWizard14);

	Sprite pGaikotsu15(Texture::Image2D::Create("Res/pGaikotsu.tga"));
	pGaikotsu15.Position(glm::vec3(-50, 110, 0));
	pGaikotsu15.Scale(glm::vec2(0));
	sprites.push_back(pGaikotsu15);

	//HP.
	Sprite barHP16(Texture::Image2D::Create("Res/Green.tga"));
	barHP16.Scale(glm::vec2(0));
	sprites.push_back(barHP16);

	//MP.
	Sprite barMP17(Texture::Image2D::Create("Res/Blue.tga"));
	barMP17.Scale(glm::vec2(0));
	sprites.push_back(barMP17);

	//�E�B�U�[�h�Ɗ[���̍U���R�}���h.
	Sprite AttackNormal18(Texture::Image2D::Create("Res/AttackTest.dds"));
	AttackNormal18.Position(glm::vec3(500, -310, 0));	//���A�C�R�������l.
	AttackNormal18.Scale(glm::vec2(0));
	sprites.push_back(AttackNormal18);

	Sprite AttackStrong19(Texture::Image2D::Create("Res/Attack2.tga"));
	AttackStrong19.Position(glm::vec3(420, -230, 0));	//���A�C�R�������l.
	AttackStrong19.Scale(glm::vec2(0));
	sprites.push_back(AttackStrong19);

	//�E�B�U�[�h�̍U���R�}���h.
	Sprite Magic20(Texture::Image2D::Create("Res/Slowing.tga"));
	Magic20.Position(glm::vec3(500, -150, 0));	//��A�C�R�������l.
	Magic20.Scale(glm::vec2(0));
	sprites.push_back(Magic20);

	Sprite Meteo21(Texture::Image2D::Create("Res/Meteo.tga"));
	Meteo21.Position(glm::vec3(580, -230, 0));	//�E�A�C�R�������l.
	Meteo21.Scale(glm::vec2(0));//Meteo33.Scale(glm::vec2(0.12f));
	sprites.push_back(Meteo21);

	//�I���A�C�R��.
	Sprite IconBlue22(Texture::Image2D::Create("Res/select.tga"));
	IconBlue22.Position(glm::vec3(-380, 210, 0));
	//IconBlue22.Scale(glm::vec2(0.8f , 0.15f));
	IconBlue22.Scale(glm::vec2(0));	//IconBlue36.Scale(glm::vec2(0.28f,0.22f));
	sprites.push_back(IconBlue22);

	//�I���A�C�R��.
	Sprite selectIcon23(Texture::Image2D::Create("Res/selectA.tga"));
	selectIcon23.Position(glm::vec3(-490, 210, 0));
	selectIcon23.Scale(glm::vec2(0));
	sprites.push_back(selectIcon23);

	//�I���A�C�R��.
	Sprite selectIcon24(Texture::Image2D::Create("Res/selectA.tga"));
	selectIcon24.Position(glm::vec3(-490, 210, 0));
	selectIcon24.Scale(glm::vec2(0));
	sprites.push_back(selectIcon24);

	//�h�q���C����HP�o�[.
	Sprite defenceHP25(Texture::Image2D::Create("Res/Green.tga"));
	defenceHP25.Scale(glm::vec2(0));
	sprites.push_back(defenceHP25);

	//�h�q���C���̃A�C�R��.
	Sprite defenceIcon26(Texture::Image2D::Create("Res/Crystal.tga"));
	defenceIcon26.Position(glm::vec3(-440, 345, 0));
	defenceIcon26.Scale(glm::vec2(0));
	sprites.push_back(defenceIcon26);

	//�G�̎c���A�C�R��.
	Sprite enemyIcon27(Texture::Image2D::Create("Res/Gobrin.tga"));
	enemyIcon27.Position(glm::vec3(290, 350, 0));
	enemyIcon27.Scale(glm::vec2(0));
	sprites.push_back(enemyIcon27);

	//�h�q���C����HP�w�i.
	Sprite defenceHP28(Texture::Image2D::Create("Res/HPBar.tga"));
	defenceHP28.Scale(glm::vec2(0));
	sprites.push_back(defenceHP28);

	//�[���̍U���A�C�R��.
	Sprite AttackBladeA29(Texture::Image2D::Create("Res/AttackBlade1.tga"));
	AttackBladeA29.Position(glm::vec3(500, -150, 0));	//��A�C�R�������l.
	AttackBladeA29.Scale(glm::vec2(0));
	sprites.push_back(AttackBladeA29);

	Sprite AttackBladeB30(Texture::Image2D::Create("Res/AttackBlade2.tga"));
	AttackBladeB30.Position(glm::vec3(580, -230, 0));	//�E�A�C�R�������l.
	AttackBladeB30.Scale(glm::vec2(0));
	sprites.push_back(AttackBladeB30);

	//�S�u�����̃A�C�R��.
	Sprite GobAttack31(Texture::Image2D::Create("Res/GobAttack.tga"));
	GobAttack31.Position(glm::vec3(500, -310, 0));	//���A�C�R�������l.
	GobAttack31.Scale(glm::vec2(0));
	sprites.push_back(GobAttack31);

	Sprite GobDash32(Texture::Image2D::Create("Res/StatusUp.tga"));
	GobDash32.Position(glm::vec3(420, -230, 0));	//���A�C�R�������l.
	GobDash32.Scale(glm::vec2(0));
	sprites.push_back(GobDash32);

	//�����g���Ȃ��A�C�R��.
	Sprite NoAttack33(Texture::Image2D::Create("Res/NoAttack.tga"));
	NoAttack33.Position(glm::vec3(500, -310, 0));	//���A�C�R�������l.
	NoAttack33.Scale(glm::vec2(0));//NoAttack50.Scale(glm::vec2(0.2f));
	sprites.push_back(NoAttack33);

	//�e�R�}���h�̃N�[���^�C��.
	Sprite IconGreen34(Texture::Image2D::Create("Res/IconGreen.tga"));
	IconGreen34.Position(glm::vec3(0, 0, 0));
	IconGreen34.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
	sprites.push_back(IconGreen34);

	Sprite IconGreen35(Texture::Image2D::Create("Res/IconGreen.tga"));
	IconGreen35.Position(glm::vec3(0, 0, 0));
	IconGreen35.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
	sprites.push_back(IconGreen35);

	Sprite IconGreen36(Texture::Image2D::Create("Res/IconGreen.tga"));
	IconGreen36.Position(glm::vec3(0, 0, 0));
	IconGreen36.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
	sprites.push_back(IconGreen36);

	Sprite IconGreen37(Texture::Image2D::Create("Res/IconGreen.tga"));
	IconGreen37.Position(glm::vec3(0, 0, 0));
	IconGreen37.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
	sprites.push_back(IconGreen37);

	Sprite MiniMap38(Texture::Image2D::Create("Res/MiniMap.tga"));
	MiniMap38.Position(glm::vec3(530, 270, 0));
	MiniMap38.Scale(glm::vec2(0));
	sprites.push_back(MiniMap38);

	Sprite PMiniIcon39(Texture::Image2D::Create("Res/PMiniIcon.tga"));
	PMiniIcon39.Scale(glm::vec2(0)); //PMiniIcon39.Scale(glm::vec2(0.02f, 0.02f));
	sprites.push_back(PMiniIcon39);

	//�~�j�}�b�v��̖h�q���C���̃A�C�R��.
	Sprite DMiniIcon40(Texture::Image2D::Create("Res/Crystal.tga"));
	DMiniIcon40.Position(glm::vec3(-250, 350, 0));
	DMiniIcon40.Scale(glm::vec2(0));
	sprites.push_back(DMiniIcon40);

	Sprite DMiniIcon41(Texture::Image2D::Create("Res/Crystal.tga"));
	DMiniIcon41.Scale(glm::vec2(0));
	sprites.push_back(DMiniIcon41);

	Sprite DMiniIcon42(Texture::Image2D::Create("Res/Crystal.tga"));
	DMiniIcon42.Scale(glm::vec2(0));
	sprites.push_back(DMiniIcon42);

	//�V�[���h�A�C�R��.
	Sprite Shield43(Texture::Image2D::Create("Res/Shield.tga"));
	Shield43.Position(glm::vec3(-500, 345, 0));
	Shield43.Scale(glm::vec2(0));
	sprites.push_back(Shield43);

	Sprite ShieldUP44(Texture::Image2D::Create("Res/ShieldUP.tga"));
	ShieldUP44.Position(glm::vec3(-480, 340, 0));
	ShieldUP44.Scale(glm::vec2(0));
	sprites.push_back(ShieldUP44);

	//�R���{������.
	Sprite Combo45(Texture::Image2D::Create("Res/Combo.tga"));
	Combo45.Scale(glm::vec2(0));
	sprites.push_back(Combo45);

	Sprite ComboMes46(Texture::Image2D::Create("Res/ComboMesseage.tga"));
	ComboMes46.Position(glm::vec3(0, -70, 0));
	ComboMes46.Scale(glm::vec2(0));
	sprites.push_back(ComboMes46);

	Sprite ComboMax47(Texture::Image2D::Create("Res/Max.tga"));
	ComboMax47.Position(glm::vec3(560, 305, 0));
	ComboMax47.Scale(glm::vec2(0));
	sprites.push_back(ComboMax47);
}

/*
*�Q�[������UI�̃T�C�Y�ύX.
*
*@param	sprites	�X�v���C�g�`��.
*@param	id		�ǂ̏�ʂŕύX���邩��ID.
*/
void SpriteRenderer::SpriteChange(std::vector<Sprite>& sprites, int selCou, int skCou, int id)
{
	if (id == 0)
	{
		sprites[1].Scale(glm::vec2(1, 3.5f));
		sprites[1].Position(glm::vec3(160, 0, 0));
		sprites[2].Scale(glm::vec2(0.4f, 2.5f));
		sprites[3].Scale(glm::vec2(0.4f, 0.9f));
		sprites[2].Position(glm::vec3(-380, 85, 0));
		sprites[3].Position(glm::vec3(-380, -220, 0));

		sprites[22].Scale(glm::vec2(1.08f, 0.11f));
		sprites[23].Scale(glm::vec2(0.1f));
		sprites[24].Scale(glm::vec2(0.1f));

		sprites[16].Scale(glm::vec2(0));
		sprites[17].Scale(glm::vec2(0));
		sprites[18].Scale(glm::vec2(0));
		sprites[19].Scale(glm::vec2(0));
		sprites[20].Scale(glm::vec2(0));
		sprites[21].Scale(glm::vec2(0));
		sprites[26].Scale(glm::vec2(0));
		sprites[27].Scale(glm::vec2(0));
		sprites[28].Scale(glm::vec2(0));
		sprites[31].Scale(glm::vec2(0));
		sprites[32].Scale(glm::vec2(0));
		sprites[29].Scale(glm::vec2(0));
		sprites[30].Scale(glm::vec2(0));
		sprites[33].Scale(glm::vec2(0));
		sprites[34].Scale(glm::vec2(0));
		sprites[35].Scale(glm::vec2(0));
		sprites[36].Scale(glm::vec2(0));
		sprites[37].Scale(glm::vec2(0));
		sprites[38].Scale(glm::vec2(0));
		sprites[39].Scale(glm::vec2(0));
		sprites[40].Scale(glm::vec2(0));
		sprites[41].Scale(glm::vec2(0));
		sprites[42].Scale(glm::vec2(0));
		sprites[43].Scale(glm::vec2(0));
		sprites[44].Scale(glm::vec2(0));
		sprites[45].Scale(glm::vec2(0));
		sprites[46].Scale(glm::vec2(0));
		sprites[47].Scale(glm::vec2(0));
	}
	else if (id == 1)
	{
		sprites[1].Scale(glm::vec2(0));
		sprites[2].Scale(glm::vec2(0));
		sprites[3].Scale(glm::vec2(0));
		sprites[12].Scale(glm::vec2(0));
		sprites[13].Scale(glm::vec2(0));
		sprites[14].Scale(glm::vec2(0));
		sprites[15].Scale(glm::vec2(0));
		sprites[16].Scale(glm::vec2(0));
		sprites[17].Scale(glm::vec2(0));
		sprites[22].Scale(glm::vec2(0));
		sprites[23].Scale(glm::vec2(0));
		sprites[24].Scale(glm::vec2(0));
		sprites[28].Scale(glm::vec2(0));
	}
	else if (id == 2)
	{
		if (skCou == 1)
		{
			//�I���A�C�R��.
			sprites[22].Position(glm::vec3(-30, 160, 0));
			sprites[23].Position(glm::vec3(80, 160, 0));
			sprites[24].Position(glm::vec3(-140, 160, 0));
		}
		else if (skCou == 2)
		{
			//�I���A�C�R��.
			sprites[22].Position(glm::vec3(-30, 110, 0));
			sprites[23].Position(glm::vec3(80, 110, 0));
			sprites[24].Position(glm::vec3(-140, 110, 0));
		}
		else if (skCou == 3)
		{
			//�I���A�C�R��.
			sprites[22].Position(glm::vec3(-30, 60, 0));
			sprites[23].Position(glm::vec3(80, 60, 0));
			sprites[24].Position(glm::vec3(-140, 60, 0));
		}
		else if (skCou == 4)
		{
			//�I���A�C�R��.
			sprites[22].Position(glm::vec3(-30, 10, 0));
			sprites[23].Position(glm::vec3(80, 10, 0));
			sprites[24].Position(glm::vec3(-140, 10, 0));
		}
	}
	else if (id == 3)
	{
		if (selCou == 1)
		{
			//�I���A�C�R��.
			sprites[22].Position(glm::vec3(-380, 210, 0));
			sprites[23].Position(glm::vec3(-270, 210, 0));
			sprites[24].Position(glm::vec3(-490, 210, 0));
		}
		else if (selCou == 2)
		{
			//�I���A�C�R��.
			sprites[22].Position(glm::vec3(-380, 160, 0));
			sprites[23].Position(glm::vec3(-270, 160, 0));
			sprites[24].Position(glm::vec3(-490, 160, 0));
		}
		else if (selCou == 3)
		{
			//�I���A�C�R��.
			sprites[22].Position(glm::vec3(-380, 110, 0));
			sprites[23].Position(glm::vec3(-270, 110, 0));
			sprites[24].Position(glm::vec3(-490, 110, 0));
		}
		else if (selCou == 4)
		{
			//�I���A�C�R��.
			sprites[22].Position(glm::vec3(-380, 10, 0));
			sprites[23].Position(glm::vec3(-270, 10, 0));
			sprites[24].Position(glm::vec3(-490, 10, 0));
		}
		else if (selCou == 5)
		{
			//�I���A�C�R��.
			sprites[22].Position(glm::vec3(-380, -40, 0));
			sprites[23].Position(glm::vec3(-270, -40, 0));
			sprites[24].Position(glm::vec3(-490, -40, 0));
		}
	}
	else if(id == 4)
	{
		sprites[2].Scale(glm::vec2(0));
		sprites[3].Scale(glm::vec2(0));
		sprites[22].Scale(glm::vec2(0));
		sprites[23].Scale(glm::vec2(0));
		sprites[24].Scale(glm::vec2(0));
	}
	else if(id == 5)
	{
		if (selCou == 0)
		{
			//�I���A�C�R��.
			sprites[24].Position(glm::vec3(82, -75, 0));
			sprites[22].Position(glm::vec3(0, -75, 0));
			sprites[23].Position(glm::vec3(-83, -75, 0));
		}
		else if (selCou == 1)
		{
			//�I���A�C�R��.
			sprites[24].Position(glm::vec3(82, -115, 0));
			sprites[22].Position(glm::vec3(0, -115, 0));
			sprites[23].Position(glm::vec3(-83, -115, 0));
		}
	}
	else if (id == 6)
	{
		if (selCou == 0)
		{
			//�I���A�C�R��.
			sprites[22].Scale(glm::vec2(1.61f, 0.22f));
			sprites[23].Scale(glm::vec2(0.2f));
			sprites[24].Scale(glm::vec2(0.2f));
			sprites[24].Position(glm::vec3(260, 120, 0));
			sprites[22].Position(glm::vec3(100, 120, 0));
			sprites[23].Position(glm::vec3(-60, 120, 0));
		}
		else if (selCou == 1)
		{
			//�I���A�C�R��.
			sprites[22].Scale(glm::vec2(1.61f, 0.22f));
			sprites[23].Scale(glm::vec2(0.2f));
			sprites[24].Scale(glm::vec2(0.2f));
			sprites[24].Position(glm::vec3(260, -120, 0));
			sprites[22].Position(glm::vec3(100, -120, 0));
			sprites[23].Position(glm::vec3(-60, -125, 0));
		}
	}
	else if (id == 7)
	{
		sprites[12].Scale(glm::vec2(0));
		sprites[13].Scale(glm::vec2(0));
		sprites[14].Scale(glm::vec2(0));
		sprites[15].Scale(glm::vec2(0));

		sprites[16].Scale(glm::vec2(0));
		sprites[17].Scale(glm::vec2(0));
	}
	else if (id == 8)
	{
	sprites[1].Scale(glm::vec2(0));
	sprites[2].Scale(glm::vec2(0));
	sprites[3].Scale(glm::vec2(0));
	sprites[12].Scale(glm::vec2(0));
	sprites[13].Scale(glm::vec2(0));
	sprites[14].Scale(glm::vec2(0));
	sprites[15].Scale(glm::vec2(0));
	sprites[16].Scale(glm::vec2(0));
	sprites[17].Scale(glm::vec2(0));
	sprites[22].Scale(glm::vec2(0));
	sprites[23].Scale(glm::vec2(0));
	sprites[24].Scale(glm::vec2(0));
	}
	else if (id == 9)
	{
	sprites[25].Scale(glm::vec2(0));
	sprites[26].Scale(glm::vec2(0));
	sprites[27].Scale(glm::vec2(0));
	sprites[43].Scale(glm::vec2(0));
	sprites[44].Scale(glm::vec2(0));
	}
	else if (id == 10)
	{
	sprites[0].Scale(glm::vec2(6.0f, 4.0f));
	sprites[1].Scale(glm::vec2(1.5f, 4.0f));
	sprites[1].Position(glm::vec3(1));

	sprites[2].Scale(glm::vec2(0));
	sprites[3].Scale(glm::vec2(0));
	sprites[16].Scale(glm::vec2(0));
	sprites[17].Scale(glm::vec2(0));
	sprites[18].Scale(glm::vec2(0));
	sprites[19].Scale(glm::vec2(0));
	sprites[20].Scale(glm::vec2(0));
	sprites[21].Scale(glm::vec2(0));
	sprites[22].Scale(glm::vec2(0));
	sprites[23].Scale(glm::vec2(0));
	sprites[24].Scale(glm::vec2(0));
	sprites[26].Scale(glm::vec2(0));
	sprites[27].Scale(glm::vec2(0));
	sprites[28].Scale(glm::vec2(0));
	sprites[31].Scale(glm::vec2(0));
	sprites[32].Scale(glm::vec2(0));
	sprites[29].Scale(glm::vec2(0));
	sprites[30].Scale(glm::vec2(0));
	sprites[33].Scale(glm::vec2(0));
	sprites[34].Scale(glm::vec2(0));
	sprites[35].Scale(glm::vec2(0));
	sprites[36].Scale(glm::vec2(0));
	sprites[37].Scale(glm::vec2(0));
	sprites[38].Scale(glm::vec2(0));
	sprites[39].Scale(glm::vec2(0));
	}
	else if (id == 11)
	{
	sprites[2].Scale(glm::vec2(0.3f, 0.7f));
	sprites[3].Scale(glm::vec2(0.9f, 0.7f));
	sprites[22].Scale(glm::vec2(0.83f, 0.11f));
	sprites[23].Scale(glm::vec2(0.1f));
	sprites[24].Scale(glm::vec2(0.1f));
	sprites[2].Position(glm::vec3(0, -100, 0));
	sprites[3].Position(glm::vec3(0, -250, 0));
	sprites[24].Position(glm::vec3(82, -75, 0));
	sprites[22].Position(glm::vec3(0, -75, 0));
	sprites[23].Position(glm::vec3(-83, -75, 0));
	}
	else if (id == 12)
	{
	sprites[11].Scale(glm::vec2(0));
	sprites[16].Scale(glm::vec2(0));
	sprites[17].Scale(glm::vec2(0));
	sprites[18].Scale(glm::vec2(0));
	sprites[19].Scale(glm::vec2(0));
	sprites[20].Scale(glm::vec2(0));
	sprites[21].Scale(glm::vec2(0));
	sprites[25].Scale(glm::vec2(0));
	sprites[26].Scale(glm::vec2(0));
	sprites[27].Scale(glm::vec2(0));
	sprites[28].Scale(glm::vec2(0));
	sprites[29].Scale(glm::vec2(0));
	sprites[30].Scale(glm::vec2(0));
	sprites[31].Scale(glm::vec2(0));
	sprites[32].Scale(glm::vec2(0));
	sprites[33].Scale(glm::vec2(0));
	sprites[34].Scale(glm::vec2(0));
	sprites[35].Scale(glm::vec2(0));
	sprites[36].Scale(glm::vec2(0));
	sprites[37].Scale(glm::vec2(0));
	sprites[38].Scale(glm::vec2(0));
	sprites[39].Scale(glm::vec2(0));
	sprites[40].Scale(glm::vec2(0));
	sprites[41].Scale(glm::vec2(0));
	sprites[42].Scale(glm::vec2(0));
	sprites[43].Scale(glm::vec2(0));
	sprites[44].Scale(glm::vec2(0));
	sprites[45].Scale(glm::vec2(0));
	sprites[46].Scale(glm::vec2(0));
	sprites[47].Scale(glm::vec2(0));

	sprites[1].Scale(glm::vec2(1, 3.5f));
	sprites[1].Position(glm::vec3(100, 0, 0));
	sprites[8].Scale(glm::vec2(1));
	sprites[9].Scale(glm::vec2(1));
	}
}

/**
*����ŃX�v���C�g�`�悵�Ă���UI�̕ύX.
*
*@param	sprites	�X�v���C�g�`��.
*@param	other	int�^�ϐ�.
*/
void SpriteRenderer::SprRootChange(std::vector<Sprite>& sprites, int other, int id)
{
	if (id == 0)
	{
		if (other == 0)
		{
			sprites[12].Scale(glm::vec2(0.25f));
		}
		else if (other == 1)
		{
			sprites[13].Scale(glm::vec2(0.3f));
		}
		else if (other == 2)
		{
			sprites[14].Scale(glm::vec2(0.3f));
		}
		else if (other == 3)
		{
			sprites[15].Scale(glm::vec2(0.3f));
		}
	}
	else if (id == 1)
	{
		if (other != 1)
		{
			sprites[26].Scale(glm::vec2(2));
			sprites[27].Scale(glm::vec2(1));
		}
	}else if(id == 2)
	{
		if (other == 0)
		{
			sprites[12].Scale(glm::vec2(0.25f));
		}
		else if (other == 1)
		{
			sprites[13].Scale(glm::vec2(0.3f));
		}
		else if (other == 2)
		{
			sprites[14].Scale(glm::vec2(0.3f));
		}
		else if (other == 3)
		{
			sprites[15].Scale(glm::vec2(0.3f));
		}
	}
}

/*
*�~�j�}�b�v�\��.
*
*@param	sprites	�`�悷��X�v���C�g.
*@param	a		�A�N�^�[���X�g.
*@param	flag	�t���O.
*@param	pos		�v���C���[�̈ʒu.
*/
void SpriteRenderer::MiniMap(std::vector<Sprite>& sprites, ActorList& a, bool flag, glm::vec3 pos)
{
	if (flag)
	{
		//�v���C���[�̃~�j�}�b�v�A�C�R��.
		sprites[38].Scale(glm::vec2(1));
		sprites[39].Scale(glm::vec2(0.02f));
		const glm::vec3 startPos(79, 0, 100);
		mapIcon.position = glm::vec3(pos.x - startPos.x, -(pos.z - startPos.z), 0) * 2.8f;
		sprites[39].Position(mapIcon.position + glm::vec3(530, 270, 0));

		//�N���X�^���̃~�j�}�b�v�A�C�R��.
		sprites[40].Scale(glm::vec2(1));
		sprites[41].Scale(glm::vec2(1));
		sprites[42].Scale(glm::vec2(1));

		const glm::vec3 x = (*(a.begin() + 0))->position;
		const glm::vec3 y = (*(a.begin() + 1))->position;
		const glm::vec3 z = (*(a.begin() + 2))->position;
		const glm::vec3 X = glm::vec3(x.x - startPos.x,
			-(x.z - startPos.z), 0) * 2.8f;
		const glm::vec3 Y = glm::vec3(y.x - startPos.x,
			-(y.z - startPos.z), 0) * 2.8f;
		const glm::vec3 Z = glm::vec3(z.x - startPos.x,
			-(z.z - startPos.z), 0) * 2.8f;
		sprites[40].Position(X + glm::vec3(530, 270, 0));
		sprites[41].Position(Y + glm::vec3(530, 270, 0));
		sprites[42].Position(Z + glm::vec3(530, 270, 0));
	}
	else
	{
		sprites[38].Scale(glm::vec2(0));
		sprites[39].Scale(glm::vec2(0));
		sprites[40].Scale(glm::vec2(0));
		sprites[41].Scale(glm::vec2(0));
		sprites[42].Scale(glm::vec2(0));
	}
}

/*
*�f�B�t�F���X�|�C���g��UI.
*
*@param	defLine	�f�B�t�F���X�|�C���g�̗̑�.
*@param	sprites	�`�悷��X�v���C�g.
*/
void SpriteRenderer::DefenceUI(float defLine, std::vector<Sprite>& sprites)
{
	sprites[26].Scale(glm::vec2(2));
	sprites[27].Scale(glm::vec2(1));

	//�h�q���C����HP�o�[�̕\��.
	//�U�����󂯂Ă��鎞.
	if (defLine >= 0)
	{
		sprites[25].Scale(glm::vec2(13.0f * defLine / 100, 0.8f));
		sprites[25].Position(glm::vec3((520 * defLine / 100) / 2 - 370, 345, 0));

		sprites[28].Scale(glm::vec2(0.4f, 0.3f));
		sprites[28].Position(glm::vec3(-80, 350, 0));
	}
	if (defLine <= 10)
	{
		sprites[25].Texture(Texture::Image2D::Create("Res/Red.tga"));
	}
	else if (defLine <= 40)
	{
		sprites[25].Texture(Texture::Image2D::Create("Res/yellow.tga"));
	}
}

/*
*�R���{��UI.
*
*@param	combo		�R���{��.
*@param	comTimer	�R���{�^�C�}�[.
*@param	comBuf		�R���{�ɉ������o�t.
*@param	sprites		�`�悷��X�v���C�g.
*/
void SpriteRenderer::comboUI(float combo,float comTimer,float comBuf, std::vector<Sprite>& sprites)
{
	if (combo > 0)
	{
		sprites[43].Scale(glm::vec2(1));
		sprites[44].Scale(glm::vec2(0.5f));
	}
	//�R���{�������ɏo��.
	if (comTimer >= 2.0f)
	{
		sprites[45].Scale(glm::vec2(0));
		sprites[46].Scale(glm::vec2(0));
	}
	else if (comTimer > 0.0f && combo <= 1)
	{
		sprites[45].Scale(glm::vec2(1.0f));
		sprites[46].Scale(glm::vec2(1.0f));
	}
	if (combo < 10)
	{
		sprites[47].Scale(glm::vec2(0));
	}
	else if (combo >= 10)
	{
		sprites[47].Scale(glm::vec2(1.0f));
	}
	for (int i = 0; i < combo; ++i)
	{
		comBuf = (i / combo) + 1.0f;
	}
}

/*
*�v���C���[�̃X�L���R�}���hUI.
*
*@param	pID		�v���C���[��ID.
*@param	pAb		�v���C���[�̃A�r���e�B���x��.
*@param	sprites	�`�悷��X�v���C�g.
*/
void SpriteRenderer::pCommandUI(int pID, int pAb,std::vector<Sprite>& sprites)
{
	//���̃X�L���R�}���h�p�l��.
	if (pID == 0)
	{
		sprites[33].Scale(glm::vec2(0.2f));

		sprites[18].Scale(glm::vec2(0));
		sprites[19].Scale(glm::vec2(0));
		sprites[20].Scale(glm::vec2(0));
		sprites[21].Scale(glm::vec2(0));
		sprites[31].Scale(glm::vec2(0));
		sprites[32].Scale(glm::vec2(0));
		sprites[29].Scale(glm::vec2(0));
		sprites[30].Scale(glm::vec2(0));
	}
	//�S�u�����̃X�L���R�}���h�p�l��.
	else if (pID == 1)
	{
		sprites[31].Scale(glm::vec2(0.2f));
		/*sprites[32].Scale(glm::vec2(0.2f));*/

		sprites[18].Scale(glm::vec2(0));
		sprites[19].Scale(glm::vec2(0));
		sprites[20].Scale(glm::vec2(0));
		sprites[21].Scale(glm::vec2(0));
		sprites[29].Scale(glm::vec2(0));
		sprites[30].Scale(glm::vec2(0));
		sprites[33].Scale(glm::vec2(0));
	}
	//�E�B�U�[�h�̃X�L���R�}���h�p�l��.
	else if (pID == 2)
	{
		sprites[18].Scale(glm::vec2(0.2f));

		if (pAb >= 4)
		{
			sprites[21].Scale(glm::vec2(0.12f));
		}
		if (pAb >= 3)
		{
			sprites[20].Scale(glm::vec2(0.2f));
		}
		if (pAb >= 2)
		{
			sprites[19].Scale(glm::vec2(0.2f));
		}

		sprites[31].Scale(glm::vec2(0));
		sprites[32].Scale(glm::vec2(0));
		sprites[29].Scale(glm::vec2(0));
		sprites[30].Scale(glm::vec2(0));
		sprites[33].Scale(glm::vec2(0));
	}
	//�[���̃X�L���R�}���h�p�l��.
	else if (pID == 3)
	{
		sprites[18].Scale(glm::vec2(0.2f));

		if (pAb >= 4)
		{
			sprites[30].Scale(glm::vec2(0.2f));
		}
		if (pAb >= 3)
		{
			sprites[29].Scale(glm::vec2(0.2f));
		}
		if (pAb >= 2)
		{
			sprites[19].Scale(glm::vec2(0.2f));
		}
		sprites[20].Scale(glm::vec2(0));
		sprites[21].Scale(glm::vec2(0));
		sprites[31].Scale(glm::vec2(0));
		sprites[32].Scale(glm::vec2(0));
		sprites[33].Scale(glm::vec2(0));
	}
}

/*
*�X�L���̃C���^�[�o����UI����.
*
*@param	sprites		�`�悷��X�v���C�g.
*@param	pAb			�v���C���[�̃A�r���e�B���x��.
*@param	pID			�v���C���[��ID.
*@param	s			���̃X�L���t���O.
*@param	e			�E�̃X�L���t���O.
*@param	n			��̃X�L���t���O.
*@param	w			���̃X�L���t���O.
*@param	eT			�E�̃X�L���^�C�}�[.
*@param	nT			��̃X�L���^�C�}�[.
*@param	wT			���̃X�L���^�C�}�[.
*@param	deltaTime	�O�񂩂�̍X�V����̌o�ߎ��ԁi�b�j..
*/
void SpriteRenderer::comIntUI(std::vector<Sprite>& sprites ,int pAb, int pID, bool s,bool e, bool n, bool w,
								float eT,float nT, float wT, float deltaTime)
{
	//�����U���������ǂ���.
	if (s == true)
	{
		//��.
		sprites[36].Scale(glm::vec2(0));
	}
	else if (s == false)
	{
		sprites[36].Scale(glm::vec2(1.55f, 1.55f));
		sprites[36].Position(glm::vec3(500, -310, 0));
	}
	//�E.
	if (pAb >= 4)
	{
		if (e == true)
		{
			eT += deltaTime;
			sprites[34].Scale(glm::vec2(1.55f, 1.55f * eT / 4.0f));
			sprites[34].Position(glm::vec3(580, 24.8f * 2.5f * eT / 4.0f / 2 - 261, 0));
		}
		else if (e == false)
		{
			sprites[34].Scale(glm::vec2(1.55f, 1.55f));
			sprites[34].Position(glm::vec3(580, -230, 0));
		}
	}
	//��.
	if (pAb >= 3)
	{
		if (n == true)
		{
			nT += deltaTime;
			sprites[37].Scale(glm::vec2(1.55f, 1.55f * nT / 3.0f));
			sprites[37].Position(glm::vec3(500, 24.8f * 2.5f * nT / 3.0f / 2 - 181, 0));
		}
		else if (n == false)
		{
			sprites[37].Scale(glm::vec2(1.55f, 1.55f));
			sprites[37].Position(glm::vec3(500, -150, 0));
		}
	}
	//��.
	if (pAb >= 2 && pID != 1)
	{
		if (w == true)
		{
			wT += deltaTime;
			sprites[35].Scale(glm::vec2(1.55f, 1.55f * wT / 2.0f));
			sprites[35].Position(glm::vec3(420, 24.8f * 2.5f * wT / 2.0f / 2 - 261, 0));
		}
		else if (w == false)
		{
			sprites[35].Scale(glm::vec2(1.55f, 1.55f));
			sprites[35].Position(glm::vec3(420, -230, 0));
		}
	}

	if (pID == 1)
	{
		sprites[34].Scale(glm::vec2(0));
		sprites[35].Scale(glm::vec2(0));
		sprites[37].Scale(glm::vec2(0));
	}
	else if (pID == 0)
	{
		sprites[34].Scale(glm::vec2(0));
		sprites[35].Scale(glm::vec2(0));
		sprites[37].Scale(glm::vec2(0));
	}

	if (wT >= 2.0f)
	{
		w = false;
		wT = 0.0f;
	}
	if (eT >= 4.0f)
	{
		e = false;
		eT = 0.0f;
	}
	if (nT >= 3.0f)
	{
		n = false;
		nT = 0.0f;
	}
}

/**
*�X�v���C�g�`��f�[�^����������.
*/

void SpriteRenderer::Clear() {

	primitives.clear();
}
