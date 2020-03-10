/**
* @file Warp.cpp
*/
#include "Warp.h"
#include "MainGameScene.h"

/**
* コンストラクタ.
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
* 衝突処理.
*/
void WarpActor::OnHit(const ActorPtr& other, const glm::vec3& p)
{
	// ギミック起動済みの場合は何もしない.
	if (isWarped) {
		return;
	}
	// 起動に成功したら起動フラグをtrueにする.
	if (parent->HandleWarpEffects(id, position)) {
		isWarped = true;
	}

}