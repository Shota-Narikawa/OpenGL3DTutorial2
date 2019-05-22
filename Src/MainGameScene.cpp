/**
*@file MainGameScene.cpp
*/
#include "MainGameScene.h"
#include "StatusScene.h"
#include "GameOverScene.h"
#include "GLFWEW.h"
#include <glm\gtc\matrix_transform.hpp>



/**
*�v���C���[�̓��͂���������.
*/

void MainGameScene::ProcessInput() {

	if (!flag) {

		flag = true;
		SceneStack::Instance().Push(std::make_shared<StatusScene>());
	}
	else {

		SceneStack::Instance().Replace(std::make_shared<GameOverScene>());
	}
}