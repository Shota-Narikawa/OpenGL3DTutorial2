/**
* @file Goal.cpp
*/
#include "Goal.h"
#include "MainGameScene.h"

/**
* �R���X�g���N�^.
*/
GoalActor::GoalActor(const Mesh::FilePtr& m, const glm::vec3& pos, int id,
	MainGameScene* p) :
	StaticMeshActor(m, "Coin", 1, pos, glm::vec3(0), glm::vec3(1)),
	id(id),
	parent(p)
{
	colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
		glm::vec3(0.4f, 0, 0), glm::vec3(0, -0.4f, 0), glm::vec3(0, 0, 0.3f), glm::vec3(0.8f, 0.8f, 0.6f));
}

/**
* �Փˏ���.
*/
void GoalActor::OnHit(const ActorPtr& other, const glm::vec3& p)
{
	// �M�~�b�N�N���ς݂̏ꍇ�͉������Ȃ�.
	if (isCointed) {
		return;
	}
	// �N���ɐ���������N���t���O��true�ɂ���.
	if (parent->HandleCoinEffects(id, position)) {
		isCointed = true;
	}
}