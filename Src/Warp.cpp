/**
* @file Warp.cpp
*/
#include "Warp.h"
#include "MainGameScene.h"

/**
* �R���X�g���N�^.
*/
WarpActor::WarpActor(const Mesh::FilePtr& m, const glm::vec3& pos, int id,
	MainGameScene* p) :
	StaticMeshActor(m, "Warp", 100, pos, glm::vec3(0), glm::vec3(1)),
	id(id),
	parent(p)
{
	colLocal = Collision::CreateCapsule(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), 0.5f);
}

/**
* �Փˏ���.
*/
void WarpActor::OnHit(const ActorPtr& other, const glm::vec3& p)
{
	// �M�~�b�N�N���ς݂̏ꍇ�͉������Ȃ�.
	if (isWarped) {
		return;
	}
	// �N���ɐ���������N���t���O��true�ɂ���.
	if (parent->HandleWarpEffects(id, position)) {
		isWarped = true;
	}

}