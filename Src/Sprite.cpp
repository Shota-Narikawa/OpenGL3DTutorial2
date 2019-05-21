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