/**
*@file.cpp
*/

#define _CRT_SECURE_NO_WARNINGS
#include "Font.h"
#include <memory>
#include <iostream>
#include <stdio.h>

/**
*�t�H���g�`��I�u�W�F�N�g������������.
*
*@param maxCharacter	�ő�`�敶����.
*
*@retval true			����������.
*@retval false			���������s.
*/

bool FontRenderer::Init(size_t maxCharacter) {

	return spriteRenderer.Init(maxCharacter, "Res/Sprite.vert", "Res/Sprite.frag");
}

/**
*�t�H���g�t�@�C����ǂݍ���.
*
*@param filename	�t�@�C����.
*
*@retval true	�ǂݍ��ݐ���.
*@retval false	�ǂݍ��ݎ��s.
*/

bool FontRenderer::LoadFromFile(const char* filename) {

	//�t�@�C�����J��.
	std::unique_ptr<FILE, decltype(&fclose)>fp(fopen(filename, "r"), &fclose);

	if (!fp) {
		std::cerr << "[�G���[]" << __func__ << ":" << filename << "���J���܂���.\n";
		return false;
	}

	//info�s��ǂݍ���.
	int line = 1;		//�ǂݍ��ލs�ԍ�(�G���[�\���ԍ�).
	int spacing[2];		//��s�ڂ̓ǂݍ��݃`�F�b�N�\.
	int ret = fscanf(fp.get(),
		"info face=\"%*[^\"]\" size=%*d bold=%*d italic=%*d charset=%*s unicode=%*d"
		" stretchH=%*d smooth=%*d aa=%*d padding=%*d,%*d,%*d,%*d spacing=%d,%d%*[^\n]",
		&spacing[0], &spacing[1]);

	if (ret < 2) {

		std::cerr << "[�G���[]" << __func__ << ":" << filename << "�̓ǂݍ��݂Ɏ��s(" <<
			line << "�s��).\n";
		return false;
	}
	++line;

	//common�s��ǂݍ���.
	float scaleH;
	ret = fscanf(fp.get(),
		" common lineHeight=%f base=%f scaleW=%*d scaleH=%f pages=%*d packed=%*d%*[^\n]",
		&lineHeight, &base, &scaleH);

	if (ret < 3) {
		std::cerr << "[�G���[]" << __func__ << ":" << filename << "�̓ǂݍ��݂Ɏ��s(" <<
			line << "�s��).\n";
		return false;
	}
	++line;

	//page�s��ǂݍ���.
	std::vector<std::string> texNameList;
	texNameList.reserve(16);

	for (;;) {

		int id;
		char tex[256];
		ret = fscanf(fp.get(), " page id=%d file=\"%255[^\"]\"", &id, tex);

		if (ret < 2) {

			break;
		}
		tex[sizeof(tex) / sizeof(tex[0]) - 1] = '\0';	//0�I�[��ۏ؂���.
		if (texNameList.size() <= static_cast<size_t>(id)) {

			texNameList.resize(id + 1);
		}
		texNameList[id] = std::string("Res/") + tex;
		++line;
	}
	if (texNameList.empty()) {

		std::cerr << "[�G���[]" << __func__ << ":" << filename << "�̓ǂݍ��݂Ɏ��s(" <<
			line << "�s��).\n";
		return false;
	}

	//chars�s��ǂݍ���.
	int charCount;	//char�s�̐�.
	ret = fscanf(fp.get(), " chars count=%d", &charCount);

	if (ret < 1) {

		std::cerr << "[�G���[]" << __func__ << ":" << filename << "�̓ǂݍ��݂Ɏ��s(" <<
			line << "�s��).\n";
		return false;
	}
	++line;

	//char�s��ǂݍ���.
	characterInfoList.clear();
	characterInfoList.resize(65536);	//16bit�ŕ\����͈͂��m��.

	for (int i = 0; i < charCount; ++i) {

		CharacterInfo info;
		ret = fscanf(fp.get(),
			" char id=%d x=%f y=%f width=%f height=%f xoffset=%f yoffset=%f xadvance=%f"
			" page=%d chnl=%*d",
			&info.id, &info.uv.x, &info.uv.y, &info.size.x, &info.size.y,
			&info.offset.x, &info.offset.y, &info.xadvance, &info.page);

		if (ret < 9) {

			std::cerr << "[�G���[]" << __func__ << ":" << filename << "�̓ǂݍ��݂Ɏ��s(" <<
				line << "�s��).\n";
			return false;
		}

		//�t�H���g�t�@�C���͍��オ���_�Ȃ̂ŁAOpenGL�̍��W�n�i���������_�j�ɕϊ�.
		info.uv.y = scaleH - info.uv.y - info.size.y;

		if (info.id < characterInfoList.size()) {

			characterInfoList[info.id] = info;
		}
		++line;
	}

	//�e�N�X�`����ǂݍ���.
	textures.clear();
	textures.reserve(texNameList.size());

	for (const std::string& e : texNameList) {

		Texture::Image2DPtr tex = Texture::Image2D::Create(e.c_str());

		if (!tex) {

			return false;
		}
		textures.push_back(tex);
	}

	return true;
}

/**
* �����F��ݒ肷��.
*
* @param c�����F.
*/
void FontRenderer::Color(const glm::vec4& c)
{
	color = c;
}

/**
*�����F���擾����.
*
* @return�����F.
*/
const glm::vec4& FontRenderer::Color() const
{
	return color;
}

/**
*������̒ǉ����J�n����.
*/

void FontRenderer::BeginUpdate() {

	spriteRenderer.BeginUpdate();
}

/**
*�������ǉ�����.
*
*@param position	�\���J�n���W�iY���W�̓t�H���g�̃x�[�X���C���j.
*@param str			�ǉ�����UF-16������.
*
*@retval true		�ǉ�����.
*@retval false		�ǉ����s.
*/

bool FontRenderer::AddString(const glm::vec2& position, const wchar_t* str) {

	glm::vec2 pos = position;

	for (const wchar_t* itr = str; *itr; ++itr) {

		const CharacterInfo& info = characterInfoList[*itr];

		if (info.id >= 0 && info.size.x && info.size.y) {

			//�X�v���C�g�̍��W�͉摜�̒��S���w�肷�邪�A�t�H���g�͍�����w�肷��.
			//�����ŁA���̍���ł��������߂̕␳�l���v�Z����.

			const float baseX = info.size.x * 0.5f + info.offset.x;
			const float baseY = base - info.size.y * 0.5f - info.offset.y;
			const glm::vec3 spritePos = glm::vec3(pos + glm::vec2(baseX, baseY), 0);

			Sprite sprite(textures[info.page]);
			sprite.Position(spritePos);
			sprite.Rectangle({ info.uv, info.size });
			sprite.Color(color);
			sprite.Scale(scale);

			if (!spriteRenderer.AddVertices(sprite)) {

				return false;
			}
		}
		pos.x += info.xadvance;	//���̕\���ʒu�ֈړ�.
	}
	return true;
}

/**
*������̒ǉ����I������.
*/
void FontRenderer::EndUpdate() {

	spriteRenderer.EndUpdate();
}

/**
*�t�H���g��`�悷��.
*
*@param screenSize	��ʃT�C�Y.
*/

void FontRenderer::Draw(const glm::vec2& screenSize) const {

	spriteRenderer.Draw(screenSize);
}

/**
*�s�̍������擾����.
*
*@return	�s�̍����i�s�N�Z�����j.
*/

float FontRenderer::LineHeight() const {

	return lineHeight;
}

/**
*�����̉������擾����.
*
*@param		c	�����𒲂ׂ镶��.
*
*@return		����c�̉���.
*/
float FontRenderer::XAdvance(wchar_t c) const
{
	if (c < 0 || c >= characterInfoList.size()) {
		return 0;
	}
	return characterInfoList[c].xadvance;
}

/*
*�Q�[�����ɕ\��������n�̃t�H���g�f�[�^��ID�ŊǗ�����.
*
*@param	fontRenderer	�\������t�H���g.
*@param	id				�\�������ʂ�ID.
*/
void GameSceneFont(FontRenderer& fontRenderer ,int id)
{
	if(id == 0)
	{
		fontRenderer.AddString(glm::vec2(90, 200), L"�\�E��");
		fontRenderer.AddString(glm::vec2(-130, -90), L"�{��̎�l���B");
		fontRenderer.AddString(glm::vec2(-130, -130), L"�G�R�Ɏ��̂�D���A���̂̂Ȃ����̑��݁B");
		fontRenderer.AddString(glm::vec2(-130, -170), L"�ނ͍Ăё̂����߂��ׂ���������B");
		fontRenderer.AddString(glm::vec2(-130, -210), L"�������ړ��ƃW�����v�����ł��Ȃ��B");
	}
	else if (id == 1)
	{
		fontRenderer.AddString(glm::vec2(90, 200), L"�S�u����");
		fontRenderer.AddString(glm::vec2(-130, -90), L"�S�u�������̈�l�B");
		fontRenderer.AddString(glm::vec2(-130, -130), L"�����̑��݂��S�u�����ƔF�߂����Ȃ���");
		fontRenderer.AddString(glm::vec2(-130, -170), L"�����̃S�u�������ł����邱�Ƃ𐾂��B");
		fontRenderer.AddString(glm::vec2(-130, -210), L"�ނ̖��͐l�Ԃ̏��ƌ������邱�Ƃł���B");
	}
	else if (id == 2)
	{
		fontRenderer.AddString(glm::vec2(90, 200), L"�E�B�U�[�h");
		fontRenderer.AddString(glm::vec2(-130, -90), L"�l�Ԃ̖����t�B");
		fontRenderer.AddString(glm::vec2(-130, -130), L"���@�̖��ƂŖ����t�Ƃ��Ĉ�Ă�ꂽ�B");
		fontRenderer.AddString(glm::vec2(-130, -170), L"�������ނ͗������ڂ���G�ɂ������悤��");
		fontRenderer.AddString(glm::vec2(-130, -210), L"�l�Ԃ������B���͂ȍU�����@��");
		fontRenderer.AddString(glm::vec2(-130, -250), L"��ɏ���g�����ߐڐ�𓾈ӂƂ���B");
	}
	else if (id == 3)
	{
		fontRenderer.AddString(glm::vec2(90, 200), L"�X�P���g��");
		fontRenderer.AddString(glm::vec2(-130, -90), L"����̍����畜�������[���R�m�B");
		fontRenderer.AddString(glm::vec2(-130, -130), L"���E��Ƃ����������G���[�g�R�m�B");
		fontRenderer.AddString(glm::vec2(-130, -170), L"����Ɠ��������ނ̎��Ə��D����");
		fontRenderer.AddString(glm::vec2(-130, -210), L"�V�ѐl�������B�ނ̎����͓D�����A����");
		fontRenderer.AddString(glm::vec2(-130, -250), L"���ւ̃Z�N�n���s�ׂɂ�莀�Y�B");
	}
	else if (id == 4)
	{
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
		fontRenderer.AddString(glm::vec2(-500, -200), L"�Ή��R�}���h��");
		fontRenderer.AddString(glm::vec2(-500, -250), L"�m�F���ł��܂�");
	}
	else if (id == 5)
	{
		fontRenderer.AddString(glm::vec2(0, 210), L"�L�[�{�[�h�R�}���h�\");
		fontRenderer.AddString(glm::vec2(-100, 150), L"WSAD:�ړ�");
		fontRenderer.AddString(glm::vec2(-100, 100), L"M�F�}�b�v�\��");
		fontRenderer.AddString(glm::vec2(-100, 50), L"U�F�X�L���P");
		fontRenderer.AddString(glm::vec2(-100, 0), L"H�F�X�L���Q");
		fontRenderer.AddString(glm::vec2(-100, -50), L"K�F�X�L���R");
		fontRenderer.AddString(glm::vec2(-100, -100), L"J�F�X�L���S");
		fontRenderer.AddString(glm::vec2(170, 150), L"A�F�J��������]");
		fontRenderer.AddString(glm::vec2(170, 100), L"D�F�J�����E��]");
		fontRenderer.AddString(glm::vec2(170, 50), L"SPACE:���j���[���");
		fontRenderer.AddString(glm::vec2(170, 0), L"ENTER:����L�[");
		fontRenderer.AddString(glm::vec2(-100, -200), L"Y�{�^���@�܂��́@U�{�^����");
		fontRenderer.AddString(glm::vec2(-100, -250), L"�Q�[���p�b�h��������ɐ؂�ւ��܂�");
	}
	else if (id == 6)
	{
		fontRenderer.AddString(glm::vec2(0, 210), L"�Q�[���p�b�h�R�}���h�\");
		fontRenderer.AddString(glm::vec2(-100, 150), L"�\���L�[:�ړ�");
		fontRenderer.AddString(glm::vec2(-100, 100), L"R�F�}�b�v�\��");
		fontRenderer.AddString(glm::vec2(-100, 50), L"Y�F�X�L���P");
		fontRenderer.AddString(glm::vec2(-100, 0), L"X�F�X�L���Q");
		fontRenderer.AddString(glm::vec2(-100, -50), L"B�F�X�L���R");
		fontRenderer.AddString(glm::vec2(-100, -100), L"A�F�X�L���S");
		fontRenderer.AddString(glm::vec2(170, 150), L"L2�F�J��������]");
		fontRenderer.AddString(glm::vec2(170, 100), L"R2�F�J�����E��]");
		fontRenderer.AddString(glm::vec2(170, 50), L"BACK:���j���[���");
		fontRenderer.AddString(glm::vec2(170, 0), L"START:����L�[");
		fontRenderer.AddString(glm::vec2(-100, -200), L"Y�{�^���@�܂��́@U�{�^����");
		fontRenderer.AddString(glm::vec2(-100, -250), L"�L�[�{�[�h��������ɐ؂�ւ��܂�");
	}
	else if (id == 7)
	{
		fontRenderer.AddString(glm::vec2(-510, -190), L"�X�L���̊m�F��");
		fontRenderer.AddString(glm::vec2(-510, -230), L"�ł��܂��B");
		fontRenderer.AddString(glm::vec2(-125, -100), L"����");
	}
	else if (id == 8)
	{
		fontRenderer.AddString(glm::vec2(-110, 200), L"�\�E��");
		fontRenderer.AddString(glm::vec2(-125, 150), L"�����ł��Ȃ�");
		fontRenderer.AddString(glm::vec2(-125, 100), L"�����ł��Ȃ�");
		fontRenderer.AddString(glm::vec2(-125, 50), L"�����ł��Ȃ�");
		fontRenderer.AddString(glm::vec2(-125, 0), L"�����ł��Ȃ�");

		fontRenderer.AddString(glm::vec2(-125, -150), L"���̂܂܂ł͉����ł��Ȃ�");
		fontRenderer.AddString(glm::vec2(-125, -200), L"���̃L�����N�^�[�ɜ߈˂��悤");
	}
	else if (id == 9)
	{
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
		fontRenderer.AddString(glm::vec2(-460, 200), L"�v���C���[");
		fontRenderer.AddString(glm::vec2(-450, 150), L"�R�}���h");
		fontRenderer.AddString(glm::vec2(-440, 100), L"�X�L��");
		fontRenderer.AddString(glm::vec2(-460, 0), L"�Q�[���I��");
		fontRenderer.AddString(glm::vec2(-440, -50), L"����");
	}
}

/*
*�����K�v�Ƃ��Ă���t�H���g�\���̊Ǘ�.
*
*@param	fontRenderer	�\������t�H���g.
*@param	skCount			�X�L���J�E���g.
*@param	playerAb		�v���C���[�̃A�r���e�B���x��.
*@param	id				�\�������ʂ�ID.
*/
void PlayerFont(FontRenderer& fontRenderer, int skCount, int playerAb, int id)
{
	if (id == 0)
	{
		fontRenderer.AddString(glm::vec2(-105, 200), L"�S�u����");
		if (skCount == 1)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"���x�Ȓ܂ő�����U��");
			fontRenderer.AddString(glm::vec2(-125, -200), L"���̒܂͊�ł��ӂ��Ă��܂�");
		}
		else if (skCount == 2)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"���������ł���悤�ɂȂ�");
			fontRenderer.AddString(glm::vec2(-125, -200), L"������");
		}
		else if (skCount == 3)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"�_�b�V�����ł���悤�ɂȂ�");
			fontRenderer.AddString(glm::vec2(-125, -200), L"������");
		}
		else if (skCount == 4)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"�����ړ����ł���悤�ɂȂ�");
			fontRenderer.AddString(glm::vec2(-125, -200), L"������");
		}
		fontRenderer.AddString(glm::vec2(-95, 150), L"�ʏ�U��");
		//�g���Ȃ��X�L���͔�����.
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
		fontRenderer.AddString(glm::vec2(-85, 100), L"������");
		fontRenderer.AddString(glm::vec2(-95, 50), L"�_�b�V��");
		fontRenderer.AddString(glm::vec2(-95, 0), L"�����ړ�");

		if (playerAb >= 4)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-95, 0), L"�����ړ�");
		}
		if (playerAb >= 3)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-95, 50), L"�_�b�V��");
		}
		if (playerAb >= 2)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-85, 100), L"������");
		}
	}
	else if (id == 1)
	{
		fontRenderer.AddString(glm::vec2(-110, 200), L"�E�B�U�[�h");
		//�X�L���̐���.
		if (skCount == 1)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"��œ˂��ߋ����U��");
			fontRenderer.AddString(glm::vec2(-125, -200), L"��͕��������������̂ł���");
		}
		else if (skCount == 2)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"���U�艺�낷�ߋ����U��");
			fontRenderer.AddString(glm::vec2(-125, -200), L"������͏�Ől������ȂƋ�����ꂽ");
		}
		else if (skCount == 3)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"�O���ɉ����@���΂��������U��");
			fontRenderer.AddString(glm::vec2(-125, -200), L"�X�s�[�h�͒x�����͈͍͂L��");
		}
		else if (skCount == 4)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"�󂩂�X���@�𗎂Ƃ��������U��");
			fontRenderer.AddString(glm::vec2(-125, -200), L"������ɂ��𗎂Ƃ��G����|����");
		}

		fontRenderer.AddString(glm::vec2(-95, 150), L"�ʏ�U��");
		//�g���Ȃ��X�L���͔�����.
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
		fontRenderer.AddString(glm::vec2(-100, 100), L"���ԂƊ���");
		fontRenderer.AddString(glm::vec2(-95, 50), L"�t�@�C�A");
		fontRenderer.AddString(glm::vec2(-95, 0), L"�u���U�h");

		if (playerAb >= 4)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-95, 0), L"�u���U�h");
		}
		if (playerAb >= 3)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-95, 50), L"�t�@�C�A");
		}
		if (playerAb >= 2)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-100, 100), L"���ԂƊ���");
		}
	}
	else if (id == 2)
	{
		fontRenderer.AddString(glm::vec2(-110, 200), L"�X�P���g��");
		//�X�L���̐���.
		if (skCount == 1)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"����U�艺�낷�ߋ����U��");
			fontRenderer.AddString(glm::vec2(-125, -200), L"�^�͎��ȗ��ł���");
		}
		else if (skCount == 2)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"�c���̓�i�K�U��");
			fontRenderer.AddString(glm::vec2(-125, -200), L"��i�K�ɂ��U���œG��|�M����");
		}
		else if (skCount == 3)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"�����悭����U�艺�낷�������U��");
			fontRenderer.AddString(glm::vec2(-125, -200), L"���ꂽ�G�������œガ����");
		}
		else if (skCount == 4)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"�������ꂽ���ɂ̒������U��");
			fontRenderer.AddString(glm::vec2(-125, -200), L"�S���ʂ̋��͂Ȉꌂ�œG����|����");
		}

		fontRenderer.AddString(glm::vec2(-95, 150), L"�ʏ�U��");
		//�g���Ȃ��X�L���͔�����.
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
		fontRenderer.AddString(glm::vec2(-95, 100), L"��i�a��");
		fontRenderer.AddString(glm::vec2(-80, 50), L"����a");
		fontRenderer.AddString(glm::vec2(-125, 0), L"�S���ʉ��Ԃ�");

		if (playerAb >= 4)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-125, 0), L"�S���ʉ��Ԃ�");
		}
		if (playerAb >= 3)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-80, 50), L"����a");
		}
		if (playerAb >= 2)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-95, 100), L"��i�a��");
		}
	}
}

/*
*�X�e�[�W�`�F���W�A���[�h��ʂ̕���p�̃t�H���g�\���̊Ǘ�.
*
*@param	fontRenderer	�\������t�H���g.
*@param	stage			�����ꂩ�̃X�e�[�W�`�F���W.
*@param	flag			�C�x���g�̃t���O..
*@param	id				�\�������ʂ�ID.
*/
void StChangeFont(FontRenderer& fontRenderer, std::vector<Sprite>& sprites, int stage, bool flag, int id)
{
	if (id == 0)
	{
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 10));
		fontRenderer.AddString(glm::vec2(280, -280), L"NowLoading...");
		//���[�h��ʒ��Ƀ����_���ŃQ�[���ɖ𗧂q���g���\�������.
		if (stage == 1)
		{
			sprites[4].Scale(glm::vec2(0.4f));
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 10));
			fontRenderer.AddString(glm::vec2(-300, -100), L"�΂����Ă���͈̂�x�N���A�����X�e�[�W�B");
			fontRenderer.AddString(glm::vec2(-300, -150), L"�����A����ł�����\�B");
		}
		else if (stage == 2)
		{
			sprites[5].Scale(glm::vec2(0.4f));
			fontRenderer.AddString(glm::vec2(-300, -100), L"��l���̓��̂̏�񂪕ۊǂ��ꂽ�N���X�^���B");
			fontRenderer.AddString(glm::vec2(-300, -150), L"�X�e�[�W���ƂɃ����_���Ŕz�u�����B");
			fontRenderer.AddString(glm::vec2(-300, -200), L"�X�e�[�W���n�܂�ΐ^����ɏꏊ��c�����悤�B");
		}
		else if (stage == 3)
		{
			sprites[6].Scale(glm::vec2(0.4f));
			fontRenderer.AddString(glm::vec2(-300, -100), L"���L���ȃL�����N�^�[�������g�p�ł���B");
			fontRenderer.AddString(glm::vec2(-300, -150), L"�X�L����v���t�B�[���̓��j���[��ʂŊm�F�\�B");
		}
		else if (stage == 4)
		{
			sprites[7].Scale(glm::vec2(0.4f));
			fontRenderer.AddString(glm::vec2(-300, -100), L"�G�͑傫���ɂ�萫�����قȂ�B");
			fontRenderer.AddString(glm::vec2(-300, -150), L"�U���͂��������́A�ړ����������̂ȂǁB");
			fontRenderer.AddString(glm::vec2(-300, -200), L"�|�����Ԃɂ͗D�揇�ʂ����悤�B");
		}
	}
	else if (id == 1)
	{
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
		fontRenderer.AddString(glm::vec2(-30, -85), L"�͂�");
		fontRenderer.AddString(glm::vec2(-45, -125), L"������");
		fontRenderer.AddString(glm::vec2(-200, -280), L"���݂܂����H");

		if (stage == 1)
		{
			fontRenderer.AddString(glm::vec2(-270, -240), L"�V�̐��FA�̎������҂��Ă���.");
			fontRenderer.AddString(glm::vec2(20, -280), L"��Փx����������");
			flag = false;
		}
		else if (stage == 2)
		{
			fontRenderer.AddString(glm::vec2(-270, -240), L"�V�̐��FB�̎������҂��Ă���.");
			fontRenderer.AddString(glm::vec2(20, -280), L"��Փx����������");
			flag = false;
		}
		else if (stage == 3)
		{
			fontRenderer.AddString(glm::vec2(-270, -240), L"�V�̐��FC�̎������҂��Ă���.");
			fontRenderer.AddString(glm::vec2(20, -280), L"��Փx����������");
			flag = false;
		}
		else if (stage == 4)
		{
			fontRenderer.AddString(glm::vec2(-270, -240), L"�V�̐��FD�̎������҂��Ă���.");
			fontRenderer.AddString(glm::vec2(20, -280), L"��Փx����������");
			flag = false;
		}
	}
}

/*
*�����t�H���g�̕\��.
*
*@param	name			�l�̏��.
*@param	fontRenderer	�\������t�H���g.
*@param	id				�\�������ʂ�ID.
*/
void NumFont(FontRenderer& fontRenderer, int name, int numTime, int id)
{
	if (id == 0)
	{
		//�X�e�[�WNo�\��.
		wchar_t str[] = L"STAGE.  ";
		int n = name;
		for (int i = 0; i < numTime; ++i)
		{
			str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
			n /= 10;
			if (n <= 0)
			{
				break;
			}
		}
		fontRenderer.AddString(glm::vec2(280, 200), str);
	}
	else if(id == 1)
	{
		//���x���\��.
		wchar_t str[] = L"Lv.   ";
		int n = name;
		for (int i = 0; i < numTime; ++i)
		{
			str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
			n /= 10;
			if (n <= 0)
			{
				break;
			}
		}
		fontRenderer.AddString(glm::vec2(100, 150), str);
	}
}