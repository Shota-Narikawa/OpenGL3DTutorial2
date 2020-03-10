/**
* @file Goal.cpp
*/
#include "Goal.h"
#include "MainGameScene.h"

/**
* コンストラクタ.
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
* 衝突処理.
*/
void GoalActor::OnHit(const ActorPtr& other, const glm::vec3& p)
{
	// ギミック起動済みの場合は何もしない.
	if (isCointed) {
		return;
	}
	// 起動に成功したら起動フラグをtrueにする.
	if (parent->HandleCoinEffects(id, position)) {
		isCointed = true;
	}
}