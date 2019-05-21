/**
*@file Sprite.cpp
*/

#include "Sprite.h"
#include <vector>
#include <iostream>
#include <glm\gtc\matrix_transform.hpp>

/**
*Spriteコンストラクタ.
*
*@param tex	スプライトとして表示するテクスチャ.
*/
Sprite::Sprite(const Texture::Image2DPtr& tex) :
	texture(tex),
	rect(Rect{ glm::vec2(),glm::vec2(tex->Width(),tex->Height()) }) {

}